// ════════════════════════════════════════════════════════════════════════════
//  RS50 Thermal Controller v3.3.5
//
//  Hardware:
//    - ESP32-S3-Zero (Waveshare)
//    - NTC 100K B3950 no eixo do motor
//    - Fan 120mm 4-pin PWM 12V
//    - Relé Songle SLA-24VDC-SL-C + driver MOSFET IRLZ44N
//    - LED(s) WS2812B (1/3/5+ configurável)
//
//  Features:
//    ✅ Estimativa adaptativa estator a partir do eixo
//    ✅ Fail-safe via relé NC (sem energia = motor ligado)
//    ✅ FAULT inteligente com grace period adaptativo
//    ✅ Contador de horas persistente (NVS)
//    ✅ LED configurável: single / bar / thermometer
//    ✅ Dashboard WiFi com WebSocket em tempo real
//    ✅ Reconexão automática WiFi
//
//  Compatibilidade:
//    - Arduino IDE 2.x
//    - ESP32 Arduino Core 2.0.14 (API ledcSetup + ledcAttachPin)
//    - FastLED 3.6.x
//    - WebSockets (Markus Sattler) 2.4.x
//
//  Licença: MIT
// ════════════════════════════════════════════════════════════════════════════

#include <Arduino.h>
#include <FastLED.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "config.h"

// ─── ESTADOS GLOBAIS ─────────────────────────────────────────────────────────
enum ThermalState {
  THERMAL_NORMAL, THERMAL_WARMING, THERMAL_WARNING,
  THERMAL_CRITICAL, THERMAL_SHUTDOWN, THERMAL_FAULT
};

float tempShaftRaw      = 25.0f;
float tempShaftFiltered = 25.0f;
float tempEstatorEst    = 25.0f;
float tempPeak          = 25.0f;
float dTdt              = 0.0f;
float tempShaftPrev     = 25.0f;
float temperature       = 25.0f;

unsigned long lastDtUpdate      = 0;
unsigned long lastUpdate        = 0;
unsigned long shutdownStartTime = 0;
unsigned long faultStartTime    = 0;
unsigned long sessionStart      = 0;
unsigned long activeMillisAccum = 0;
unsigned long lastHourSave      = 0;
unsigned long lastWsUpdate      = 0;
unsigned long lastWifiRetry     = 0;

int      pwmDuty         = PWM_MIN;
bool     sensorFault     = false;
bool     faultLatched    = false;
bool     relayShutdown   = false;
uint32_t totalActiveHours= 0;

ThermalState thermalState = THERMAL_NORMAL;

CRGB leds[LED_COUNT];
Preferences prefs;
WebServer server(WEB_PORT);
WebSocketsServer webSocket(WS_PORT);

// ════════════════════════════════════════════════════════════════════════════
//  MÓDULO 1: LEITURA E PROCESSAMENTO TÉRMICO
// ════════════════════════════════════════════════════════════════════════════

float readTemperatureRaw() {
  long adcSum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    adcSum += analogRead(PIN_NTC);
    delayMicroseconds(100);
  }
  float adcValue = adcSum / (float)SAMPLES;

  if (adcValue <= ADC_FAULT_LO || adcValue >= ADC_FAULT_HI) {
    if (!sensorFault) {
      sensorFault = true;
      faultStartTime = millis();
      Serial.printf("⚠️ NTC FAULT detectado (ADC=%.0f)\n", adcValue);
    }
    return NAN;
  }

  if (sensorFault) {
    sensorFault = false;
    faultStartTime = 0;
    faultLatched = false;
    Serial.println(F("✅ NTC voltou a responder"));
  }

  float voltage    = (adcValue / ADC_RES) * VCC_ADC;
  float resistance = R_FIXED * (voltage / (VCC_ADC - voltage));
  float steinhart  = resistance / R_NTC_25C;
  steinhart = log(steinhart);
  steinhart /= NTC_BETA;
  steinhart += 1.0f / T_NOMINAL_K;
  steinhart = 1.0f / steinhart;
  steinhart -= 273.15f;
  return steinhart;
}

void updateDerivative(float currentTemp) {
  unsigned long now = millis();
  unsigned long dt_ms = now - lastDtUpdate;
  if (dt_ms >= DTDT_WINDOW_MS) {
    dTdt = (currentTemp - tempShaftPrev) * 1000.0f / dt_ms;
    tempShaftPrev = currentTemp;
    lastDtUpdate = now;
  }
}

float estimateStatorTemp(float shaftTemp) {
  float estator = shaftTemp + THERMAL_OFFSET;
  if (dTdt > DTDT_TRIGGER) estator += dTdt * LEAD_TIME_S;
  return estator;
}

void readAndProcessTemp() {
  float raw = readTemperatureRaw();
  if (isnan(raw)) return;
  tempShaftRaw = raw;
  tempShaftFiltered = EMA_ALPHA * raw + (1.0f - EMA_ALPHA) * tempShaftFiltered;
  updateDerivative(tempShaftFiltered);
  tempEstatorEst = estimateStatorTemp(tempShaftFiltered);
  temperature = tempEstatorEst;
  if (temperature > tempPeak) tempPeak = temperature;
}

unsigned long getFaultGracePeriod() {
  if (tempEstatorEst < 50.0f)      return FAULT_GRACE_LOW;
  else if (tempEstatorEst < 55.0f) return FAULT_GRACE_MID;
  else                              return FAULT_GRACE_HIGH;
}

ThermalState evaluateThermalState(float temp) {
  if (sensorFault) {
    unsigned long grace = getFaultGracePeriod();
    if (millis() - faultStartTime >= grace) {
      faultLatched = true;
      return THERMAL_FAULT;
    }
    return THERMAL_CRITICAL;  // durante grace: força fan 100%
  }
  if (temp >= TEMP_SHUTDOWN)    return THERMAL_SHUTDOWN;
  if (temp >= TEMP_CRITICAL)    return THERMAL_CRITICAL;
  if (temp >= TEMP_WARNING)     return THERMAL_WARNING;
  if (temp >= TEMP_MIN + 5)     return THERMAL_WARMING;
  return THERMAL_NORMAL;
}

// ════════════════════════════════════════════════════════════════════════════
//  MÓDULO 2: CONTROLE DO RELÉ (FAIL-SAFE NC)
// ════════════════════════════════════════════════════════════════════════════

void updateRelay() {
  // Aciona shutdown
  if ((thermalState == THERMAL_SHUTDOWN ||
       (thermalState == THERMAL_FAULT && faultLatched))
      && !relayShutdown) {
    relayShutdown = true;
    shutdownStartTime = millis();
    Serial.println(F("\n╔══════════════════════════════════════════════════╗"));
    Serial.println(F("║  ⚠️  EMERGÊNCIA — MOTOR DESLIGADO  ⚠️             ║"));
    Serial.printf( "║  Motivo: %-39s ║\n",
                   thermalState == THERMAL_FAULT ? "Sensor falhou"
                                                  : "Temperatura crítica");
    Serial.println(F("║  Aguarde resfriamento (mínimo 60s)               ║"));
    Serial.println(F("╚══════════════════════════════════════════════════╝"));
  }

  // Religa se OK
  if (relayShutdown) {
    bool tempOK       = (temperature < TEMP_RECOVERY) && !sensorFault;
    bool latchExpired = (millis() - shutdownStartTime) >= SHUTDOWN_LATCH_MS;
    if (tempOK && latchExpired) {
      relayShutdown = false;
      Serial.println(F("✅ Sistema religado — temperatura normalizada"));
    }
  }

  // Aplica no GPIO (fail-safe NC: GPIO LOW = motor ON)
  bool motorEnabled = !relayShutdown;
  bool gpioState;
  if (RELAY_FAILSAFE_NC) {
    gpioState = motorEnabled ? LOW : HIGH;
  } else {
    gpioState = motorEnabled ? HIGH : LOW;
  }
  digitalWrite(PIN_RELAY, gpioState);
}

// ════════════════════════════════════════════════════════════════════════════
//  MÓDULO 3: CONTROLE PWM DA FAN
// ════════════════════════════════════════════════════════════════════════════

void updatePWM() {
  if (thermalState >= THERMAL_CRITICAL) {
    pwmDuty = PWM_MAX;
    ledcWrite(PWM_CHANNEL_FAN, 255);
    return;
  }
  static float tempLastApplied = 25.0f;
  if (fabs(temperature - tempLastApplied) < TEMP_HYSTERESIS) return;
  tempLastApplied = temperature;

  if (temperature <= TEMP_MIN)      pwmDuty = PWM_MIN;
  else if (temperature >= TEMP_MAX) pwmDuty = PWM_MAX;
  else pwmDuty = PWM_MIN + (PWM_MAX - PWM_MIN) *
                  (temperature - TEMP_MIN) / (TEMP_MAX - TEMP_MIN);
  pwmDuty = constrain(pwmDuty, PWM_MIN, PWM_MAX);
  ledcWrite(PWM_CHANNEL_FAN, (pwmDuty * 255) / 100);
}

// ════════════════════════════════════════════════════════════════════════════
//  MÓDULO 4: LEDs ENDEREÇÁVEIS (modo configurável)
// ════════════════════════════════════════════════════════════════════════════

CRGB getStateColor(ThermalState s) {
  switch (s) {
    case THERMAL_NORMAL:   return CRGB(0, 255, 0);
    case THERMAL_WARMING:  return CRGB(0, 200, 100);
    case THERMAL_WARNING:  return CRGB(255, 140, 0);
    case THERMAL_CRITICAL: return CRGB(255, 0, 0);
    case THERMAL_SHUTDOWN: return CRGB(255, 0, 0);
    case THERMAL_FAULT:    return CRGB(255, 0, 200);
    default:               return CRGB::Black;
  }
}

uint8_t getStateBrightness(ThermalState s) {
  switch (s) {
    case THERMAL_NORMAL:  return LED_BRIGHTNESS_NORMAL;
    case THERMAL_WARMING:
    case THERMAL_WARNING: return LED_BRIGHTNESS_INFO;
    case THERMAL_CRITICAL:
    case THERMAL_SHUTDOWN:
    case THERMAL_FAULT:   return LED_BRIGHTNESS_MAX;
    default:              return LED_BRIGHTNESS_MAX;
  }
}

// Mapeia temperatura para gradiente verde→amarelo→vermelho
CRGB tempToColor(float t) {
  // 30°C = verde, 50°C = amarelo, 70°C = vermelho
  float pct = constrain((t - 30.0f) / 40.0f, 0.0f, 1.0f);
  uint8_t hue = (1.0f - pct) * 96;  // 96=verde, 0=vermelho no HSV FastLED
  return CHSV(hue, 255, 255);
}

void renderLEDs_Single() {
  // Modo 1 LED: cor por estado, com piscadas em estados altos
  static unsigned long lastUpd = 0;
  static bool blinkOn = true;
  unsigned long now = millis();

  CRGB color = getStateColor(thermalState);
  uint8_t br = getStateBrightness(thermalState);
  unsigned long interval = 0;
  bool blink = false;

  if (thermalState == THERMAL_CRITICAL) { blink = true; interval = CRITICAL_BLINK_MS; }
  else if (thermalState == THERMAL_SHUTDOWN) { blink = true; interval = SHUTDOWN_STROBE_MS; }
  else if (thermalState == THERMAL_FAULT)    { blink = true; interval = FAULT_STROBE_MS; }

  if (blink) {
    if (now - lastUpd >= interval) {
      blinkOn = !blinkOn;
      leds[0] = blinkOn ? color : CRGB::Black;
      if (blinkOn) leds[0].nscale8(br);
      FastLED.show();
      lastUpd = now;
    }
  } else {
    leds[0] = color;
    leds[0].nscale8(br);
    if (now - lastUpd >= 100) {
      FastLED.show();
      lastUpd = now;
    }
  }
}

void renderLEDs_Bar() {
  // Modo 3-4 LEDs: [estado][temperatura][fan]
  static unsigned long lastUpd = 0;
  unsigned long now = millis();
  if (now - lastUpd < 200) return;
  lastUpd = now;

  // LED 0: estado geral
  leds[0] = getStateColor(thermalState);
  leds[0].nscale8(getStateBrightness(thermalState));

  // LED 1: temperatura como gradiente
  if (LED_COUNT >= 2) {
    leds[1] = tempToColor(temperature);
    leds[1].nscale8(LED_BRIGHTNESS_INFO);
  }

  // LED 2: PWM da fan (azul = baixo, branco = alto)
  if (LED_COUNT >= 3) {
    uint8_t fanBr = map(pwmDuty, PWM_MIN, PWM_MAX, 30, 255);
    leds[2] = CRGB(fanBr, fanBr, 255);
    leds[2].nscale8(LED_BRIGHTNESS_INFO);
  }

  // Se tiver 4º LED: pico da sessão
  if (LED_COUNT >= 4) {
    leds[3] = tempToColor(tempPeak);
    leds[3].nscale8(LED_BRIGHTNESS_INFO);
  }

  // Pisca em emergência
  if (thermalState >= THERMAL_CRITICAL) {
    static bool flash = false;
    flash = !flash;
    if (flash) for (int i = 0; i < LED_COUNT; i++) leds[i] = CRGB::Red;
  }
  FastLED.show();
}

void renderLEDs_Thermometer() {
  // Modo 5+ LEDs: termômetro vertical
  static unsigned long lastUpd = 0;
  unsigned long now = millis();
  if (now - lastUpd < 100) return;
  lastUpd = now;

  // Mapeia temperatura para "altura" preenchida
  float pct = constrain((temperature - 25.0f) / (TEMP_SHUTDOWN - 25.0f), 0.0f, 1.0f);
  int filled = round(pct * LED_COUNT);

  for (int i = 0; i < LED_COUNT; i++) {
    if (i < filled) {
      // Cada LED tem sua cor fixa baseada na posição
      float ledTemp = 25.0f + ((float)i / LED_COUNT) * (TEMP_SHUTDOWN - 25.0f);
      leds[i] = tempToColor(ledTemp);
      leds[i].nscale8(LED_BRIGHTNESS_INFO);
    } else {
      leds[i] = CRGB::Black;
    }
  }

  // Estados altos: piscadas no LED do topo
  if (thermalState == THERMAL_CRITICAL) {
    bool flash = (now % 500 < 250);
    if (flash) leds[LED_COUNT - 1] = CRGB::Red;
  }
  if (thermalState == THERMAL_SHUTDOWN) {
    bool strobe = (now % 160 < 80);
    for (int i = 0; i < LED_COUNT; i++) leds[i] = strobe ? CRGB::Red : CRGB::Black;
  }
  if (thermalState == THERMAL_FAULT) {
    for (int i = 0; i < LED_COUNT; i++) leds[i] = CRGB(255, 0, 200);
    if ((now / 200) % 2) FastLED.clear();
  }
  FastLED.show();
}

void updateLEDs() {
  if (LED_MODE_SINGLE)            renderLEDs_Single();
  else if (LED_MODE_BAR)          renderLEDs_Bar();
  else if (LED_MODE_THERMOMETER)  renderLEDs_Thermometer();
}

// ════════════════════════════════════════════════════════════════════════════
//  MÓDULO 5: CONTADOR DE HORAS (persistente)
// ════════════════════════════════════════════════════════════════════════════

void updateHourCounter() {
  static unsigned long lastTick = 0;
  unsigned long now = millis();

  if (tempShaftFiltered > ACTIVE_THRESHOLD_TEMP) {
    if (lastTick > 0) activeMillisAccum += (now - lastTick);
    lastTick = now;
  } else {
    lastTick = 0;
  }

  if (now - lastHourSave >= HOUR_COUNTER_SAVE_INTERVAL) {
    lastHourSave = now;
    if (activeMillisAccum >= 60000) {
      uint32_t newMinutes = activeMillisAccum / 60000;
      activeMillisAccum %= 60000;
      uint32_t totalMin = prefs.getUInt("totalMin", 0) + newMinutes;
      prefs.putUInt("totalMin", totalMin);
      totalActiveHours = totalMin / 60;
      Serial.printf("💾 Total acumulado: %u h %u min\n",
                    totalActiveHours, totalMin % 60);
    }
  }
}

// ════════════════════════════════════════════════════════════════════════════
//  MÓDULO 6: WIFI + DASHBOARD WEB
// ════════════════════════════════════════════════════════════════════════════

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>RS50 Thermal Monitor</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:'Segoe UI',system-ui,sans-serif;background:#0a0a0f;color:#e0e0e0;min-height:100vh;padding:20px}
.container{max-width:900px;margin:0 auto}
h1{font-size:1.5em;margin-bottom:5px;color:#4af}
.subtitle{font-size:0.85em;color:#888;margin-bottom:20px}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:15px;margin-bottom:20px}
.card{background:#15151f;border-radius:12px;padding:20px;border:1px solid #2a2a3a;transition:all 0.3s}
.card:hover{border-color:#4af}
.card-label{font-size:0.85em;color:#888;text-transform:uppercase;letter-spacing:0.5px}
.card-value{font-size:2.2em;font-weight:700;margin-top:5px;line-height:1}
.card-unit{font-size:0.5em;color:#888;margin-left:5px}
.state{font-size:1.4em;font-weight:600;padding:10px 18px;border-radius:8px;display:inline-block;margin-top:8px}
.NORMAL{background:#1a4a1a;color:#6f6}
.WARMING{background:#1a3a4a;color:#6cf}
.WARNING{background:#4a3a1a;color:#fc6}
.CRITICAL{background:#4a1a1a;color:#f66;animation:blink 0.5s infinite}
.SHUTDOWN{background:#6a0000;color:#fff;animation:blink 0.2s infinite}
.FAULT{background:#4a1a4a;color:#f6f;animation:blink 0.4s infinite}
@keyframes blink{50%{opacity:0.4}}
.bar{width:100%;height:24px;background:#222;border-radius:12px;overflow:hidden;margin-top:10px}
.bar-fill{height:100%;transition:width 0.5s,background 0.3s;background:linear-gradient(90deg,#4af 0%,#4f4 30%,#fc4 60%,#f44 100%)}
.footer{text-align:center;color:#555;font-size:0.8em;margin-top:30px}
.connection{position:fixed;top:10px;right:10px;padding:5px 10px;border-radius:5px;font-size:0.8em}
.online{background:#1a4a1a;color:#6f6}
.offline{background:#4a1a1a;color:#f66}
</style></head><body>
<div class="container">
<h1>🏎️ RS50 Thermal Monitor</h1>
<div class="subtitle">DD Wheel + ODESC FFBeast | Firmware <span id="fw">--</span></div>
<div id="conn" class="connection offline">Conectando...</div>
<div class="grid">
<div class="card"><div class="card-label">Estado</div><div id="state" class="state NORMAL">--</div></div>
<div class="card"><div class="card-label">Temperatura Eixo</div><div class="card-value"><span id="tShaft">--</span><span class="card-unit">°C</span></div></div>
<div class="card"><div class="card-label">Estator Estimado</div><div class="card-value"><span id="tStator">--</span><span class="card-unit">°C</span></div><div class="bar"><div id="barStator" class="bar-fill" style="width:0%"></div></div></div>
<div class="card"><div class="card-label">Taxa Aquecimento</div><div class="card-value"><span id="dtdt">--</span><span class="card-unit">°C/min</span></div></div>
<div class="card"><div class="card-label">Fan PWM</div><div class="card-value"><span id="pwm">--</span><span class="card-unit">%</span></div><div class="bar"><div id="barPwm" class="bar-fill" style="width:0%"></div></div></div>
<div class="card"><div class="card-label">Pico da Sessão</div><div class="card-value"><span id="peak">--</span><span class="card-unit">°C</span></div></div>
<div class="card"><div class="card-label">Motor</div><div class="card-value" id="motor">--</div></div>
<div class="card"><div class="card-label">Horas Totais</div><div class="card-value"><span id="hours">--</span><span class="card-unit">h</span></div></div>
</div>
<div class="footer">RS50 Thermal Controller v<span id="fwFooter">--</span> © 2026</div>
</div>
<script>
let ws;
function connect(){
  ws = new WebSocket(`ws://${location.hostname}:81`);
  ws.onopen = () => {
    document.getElementById('conn').className='connection online';
    document.getElementById('conn').textContent='● Online';
  };
  ws.onclose = () => {
    document.getElementById('conn').className='connection offline';
    document.getElementById('conn').textContent='● Offline (reconectando...)';
    setTimeout(connect, 2000);
  };
  ws.onmessage = (e) => {
    const d = JSON.parse(e.data);
    document.getElementById('fw').textContent = d.fw;
    document.getElementById('fwFooter').textContent = d.fw;
    document.getElementById('state').textContent = d.state;
    document.getElementById('state').className = 'state ' + d.state;
    document.getElementById('tShaft').textContent = d.tShaft.toFixed(1);
    document.getElementById('tStator').textContent = d.tStator.toFixed(1);
    document.getElementById('dtdt').textContent = (d.dtdt*60).toFixed(2);
    document.getElementById('pwm').textContent = d.pwm;
    document.getElementById('peak').textContent = d.peak.toFixed(1);
    document.getElementById('motor').textContent = d.motor ? '✅ ON' : '❌ OFF';
    document.getElementById('hours').textContent = d.hours;
    document.getElementById('barStator').style.width = Math.min(100,(d.tStator/70)*100)+'%';
    document.getElementById('barPwm').style.width = d.pwm+'%';
  };
}
connect();
</script></body></html>
)rawliteral";

void sendTelemetry() {
  String stateStr;
  switch (thermalState) {
    case THERMAL_NORMAL:   stateStr = "NORMAL";   break;
    case THERMAL_WARMING:  stateStr = "WARMING";  break;
    case THERMAL_WARNING:  stateStr = "WARNING";  break;
    case THERMAL_CRITICAL: stateStr = "CRITICAL"; break;
    case THERMAL_SHUTDOWN: stateStr = "SHUTDOWN"; break;
    case THERMAL_FAULT:    stateStr = "FAULT";    break;
  }
  String json = "{";
  json += "\"fw\":\"" FW_VERSION "\",";
  json += "\"state\":\"" + stateStr + "\",";
  json += "\"tShaft\":" + String(tempShaftFiltered, 1) + ",";
  json += "\"tStator\":" + String(tempEstatorEst, 1) + ",";
  json += "\"dtdt\":" + String(dTdt, 3) + ",";
  json += "\"pwm\":" + String(pwmDuty) + ",";
  json += "\"peak\":" + String(tempPeak, 1) + ",";
  json += "\"motor\":" + String(relayShutdown ? "false" : "true") + ",";
  json += "\"hours\":" + String(totalActiveHours);
  json += "}";
  webSocket.broadcastTXT(json);
}

void setupWiFi() {
  if (!ENABLE_WIFI) return;
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print(F("Conectando WiFi"));
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    delay(300); Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n✅ WiFi conectado: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("   Acesse: http://%s.local OU http://%s\n",
                  WIFI_HOSTNAME, WiFi.localIP().toString().c_str());
    server.on("/", []() { server.send_P(200, "text/html", HTML_PAGE); });
    server.begin();
    webSocket.begin();
  } else {
    Serial.println(F("\n⚠️ WiFi falhou — modo offline (tentará reconectar)"));
  }
}

void wifiReconnect() {
  if (!ENABLE_WIFI) return;
  if (WiFi.status() == WL_CONNECTED) return;
  unsigned long now = millis();
  if (now - lastWifiRetry < WIFI_RETRY_MS) return;
  lastWifiRetry = now;
  Serial.println(F("🔄 Tentando reconectar WiFi..."));
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

// ════════════════════════════════════════════════════════════════════════════
//  MÓDULO 7: TELEMETRIA SERIAL
// ════════════════════════════════════════════════════════════════════════════

void printTelemetry() {
  const char* state = "?";
  const char* icon  = "?";
  switch (thermalState) {
    case THERMAL_NORMAL:   state = "NORMAL";   icon = "✅"; break;
    case THERMAL_WARMING:  state = "WARMING";  icon = "🌡️ "; break;
    case THERMAL_WARNING:  state = "WARNING";  icon = "⚠️ "; break;
    case THERMAL_CRITICAL: state = "CRITICAL"; icon = "🔥"; break;
    case THERMAL_SHUTDOWN: state = "SHUTDOWN"; icon = "☠️ "; break;
    case THERMAL_FAULT:    state = "FAULT";    icon = "❌"; break;
  }
  Serial.printf("%s %-9s | Eixo:%5.1f | Estator~%5.1f | dT:%+5.2f°C/min | Fan:%3d%% | Motor:%s | Pico:%.1f | %uh\n",
    icon, state, tempShaftFiltered, tempEstatorEst,
    dTdt * 60.0f, pwmDuty, relayShutdown ? "OFF" : "ON ",
    tempPeak, totalActiveHours);
}

// ════════════════════════════════════════════════════════════════════════════
//  SETUP & LOOP
// ════════════════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(500);

  // Relé inicia DESLIGADO no boot (precaução)
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, RELAY_FAILSAFE_NC ? HIGH : LOW);

  // PWM — API do ESP32 Arduino Core 2.0.x
  ledcSetup(PWM_CHANNEL_FAN, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PIN_PWM, PWM_CHANNEL_FAN);
  ledcWrite(PWM_CHANNEL_FAN, 255);  // boot test fan 100%

  // ADC (Core 2.x: ADC_11db é a constante correta)
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // LEDs
  FastLED.addLeds<LED_TYPE, PIN_LED_RGB, LED_COLOR_ORDER>(leds, LED_COUNT);
  FastLED.setBrightness(255);
  fill_solid(leds, LED_COUNT, CRGB::Blue);
  FastLED.show();

  // NVS
  prefs.begin("thermal", false);
  totalActiveHours = prefs.getUInt("totalMin", 0) / 60;

  Serial.println(F("\n╔══════════════════════════════════════════════════╗"));
  Serial.printf( "║  %-48s ║\n", FW_NAME " v" FW_VERSION);
  Serial.println(F("║  RS50 Clone + Hoverboard 15Nm + ODESC FFBeast    ║"));
  Serial.println(F("╚══════════════════════════════════════════════════╝"));
  Serial.printf("📊 LED mode: %s (%d LEDs)\n",
    LED_MODE_SINGLE ? "SINGLE" :
    LED_MODE_BAR ? "BAR" : "THERMOMETER", LED_COUNT);
  Serial.printf("⏱️  Total acumulado: %u horas\n", totalActiveHours);
  Serial.println(F("🌀 Boot test: fan 100% por 2s..."));
  delay(2000);

  // Habilita motor
  digitalWrite(PIN_RELAY, RELAY_FAILSAFE_NC ? LOW : HIGH);
  Serial.println(F("✅ Motor habilitado.\n"));

  setupWiFi();
  sessionStart = millis();
}

void loop() {
  unsigned long now = millis();

  if (now - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = now;
    readAndProcessTemp();
    thermalState = evaluateThermalState(temperature);
    updatePWM();
    updateRelay();
    updateHourCounter();
    printTelemetry();
  }

  if (ENABLE_WIFI) {
    if (WiFi.status() == WL_CONNECTED) {
      server.handleClient();
      webSocket.loop();
      if (now - lastWsUpdate >= WS_UPDATE_MS) {
        lastWsUpdate = now;
        sendTelemetry();
      }
    } else {
      wifiReconnect();
    }
  }

  updateLEDs();
}
