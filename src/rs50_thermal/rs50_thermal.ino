/*
 * ============================================================================
 *  RS50 Thermal Controller
 *  Firmware: rs50_thermal.ino
 *  Version : 3.3.12
 *  Target  : ESP32 (Joinville/BR)
 *  Author  : Delano
 * ----------------------------------------------------------------------------
 *  Features:
 *   - NTC reading w/ Steinhart-Hart + EMA filter
 *   - Thermal FSM (IDLE / WARMING / WARNING / CRITICAL) w/ hysteresis
 *   - PWM fan control (LEDC, GPIO5, 25kHz)
 *   - Fail-safe relay (GPIO4, active HIGH) engaged only on CRITICAL
 *   - WS2812 status strip (GPIO9, 5 LEDs)
 *   - WebSocket + UDP telemetry
 *   - HTTP dashboard (dashboard.h)
 *   - OTA with visual fail-safe (magenta start / white done / red error)
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFiUdp.h>
#include <FastLED.h>
#include <ArduinoOTA.h>
#include <math.h>

#include "config.h"
#include "dashboard.h"

#define FW_VERSION "3.3.12"

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
CRGB leds[LED_COUNT];
WebServer       server(80);
WebSocketsServer ws(81);
WiFiUDP         udp;

enum ThermalState { IDLE, WARMING, WARNING, CRITICAL };
ThermalState state = IDLE;

float    currentTemp   = TEMP_IDLE;
float    filteredTemp  = TEMP_IDLE;
uint8_t  currentDuty   = PWM_MIN;
bool     relayOn       = false;

unsigned long lastSample    = 0;
unsigned long lastTelemetry = 0;

const uint8_t PWM_CHANNEL = PWM_CHANNEL_FAN;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
const char* stateName(ThermalState s) {
  switch (s) {
    case IDLE:     return "IDLE";
    case WARMING:  return "WARMING";
    case WARNING:  return "WARNING";
    case CRITICAL: return "CRITICAL";
    default:       return "UNKNOWN";
  }
}

void printBanner() {
  Serial.println();
  Serial.println(F("============================================"));
  Serial.println(F(" RS50 Thermal Controller"));
  Serial.printf ("  Firmware : %s\n", FW_VERSION);
  Serial.printf ("  Build    : %s %s\n", __DATE__, __TIME__);
  Serial.printf ("  Chip ID  : %04X%08X\n",
                 (uint16_t)(ESP.getEfuseMac() >> 32),
                 (uint32_t) ESP.getEfuseMac());
  Serial.println(F("============================================"));
}

// ---------------------------------------------------------------------------
// LEDs
// ---------------------------------------------------------------------------
void setupLEDs() {
  FastLED.addLeds<WS2812, PIN_WS2812, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(LED_BRIGHTNESS);
  fill_solid(leds, LED_COUNT, CRGB::Black);
  FastLED.show();
}

void updateLEDs(ThermalState s) {
  CRGB c;
  switch (s) {
    case IDLE:     c = CRGB::Blue;   break;
    case WARMING:  c = CRGB::Green;  break;
    case WARNING:  c = CRGB::Yellow; break;
    case CRITICAL: c = CRGB::Red;    break;
  }
  fill_solid(leds, LED_COUNT, c);
  FastLED.show();
}

// ---------------------------------------------------------------------------
// PWM
// ---------------------------------------------------------------------------
void setupPWM() {
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PIN_PWM, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0);
}

void applyDuty(uint8_t duty) {
  if (duty == 0) { ledcWrite(PWM_CHANNEL, 0); return; }
  uint32_t maxVal = (1 << PWM_RESOLUTION) - 1;
  uint32_t raw    = (duty * maxVal) / 100;
  ledcWrite(PWM_CHANNEL, raw);
}

uint8_t computeDuty(float t) {
  if (t <= TEMP_IDLE)     return PWM_MIN;
  if (t >= TEMP_CRITICAL) return PWM_MAX;
  float ratio = (t - TEMP_IDLE) / (TEMP_CRITICAL - TEMP_IDLE);
  return PWM_MIN + (uint8_t)(ratio * (PWM_MAX - PWM_MIN));
}

// ---------------------------------------------------------------------------
// NTC
// ---------------------------------------------------------------------------
float readNTC() {
  const int samples = 8;
  uint32_t acc = 0;
  for (int i = 0; i < samples; i++) {
    acc += analogRead(PIN_NTC);
    delayMicroseconds(200);
  }
  float adc = acc / (float)samples;
  if (adc <= 0) adc = 1;
  float r    = NTC_SERIES_R * (ADC_MAX - adc) / adc;
  float lnR  = logf(r / NTC_NOMINAL_R);
  float invT = (1.0f / NTC_NOMINAL_T) + (lnR / NTC_BETA);
  float tK   = 1.0f / invT;
  return tK - 273.15f + THERMAL_OFFSET;
}

// ---------------------------------------------------------------------------
// FSM
// ---------------------------------------------------------------------------
ThermalState computeState(float temp, ThermalState prev) {
  const float H = THERMAL_HYSTERESIS;
  switch (prev) {
    case IDLE:
      if (temp > TEMP_WARMING + H)   return WARMING;
      break;
    case WARMING:
      if (temp > TEMP_WARNING + H)   return WARNING;
      if (temp < TEMP_IDLE   - H)    return IDLE;
      break;
    case WARNING:
      if (temp > TEMP_CRITICAL + H)  return CRITICAL;
      if (temp < TEMP_WARNING  - H)  return WARMING;
      break;
    case CRITICAL:
      if (temp < TEMP_CRITICAL - H)  return WARNING;
      break;
  }
  return prev;
}

// ---------------------------------------------------------------------------
// Telemetry
// ---------------------------------------------------------------------------
void sendTelemetry() {
  char json[224];
  snprintf(json, sizeof(json),
    "{\"fw\":\"%s\",\"temp\":%.2f,\"duty\":%u,\"state\":\"%s\",\"relay\":%s,\"uptime\":%lu}",
    FW_VERSION, filteredTemp, currentDuty, stateName(state),
    relayOn ? "true" : "false", millis() / 1000);
  ws.broadcastTXT(json);

  char csv[96];
  snprintf(csv, sizeof(csv), "%.2f,%u,%s,%d,%lu\n",
           filteredTemp, currentDuty, stateName(state),
           relayOn ? 1 : 0, millis() / 1000);
  udp.beginPacket(UDP_HOST, UDP_PORT);
  udp.print(csv);
  udp.endPacket();
}

// ---------------------------------------------------------------------------
// HTTP handlers
// ---------------------------------------------------------------------------
void handleRoot()   { server.send_P(200, "text/html", DASHBOARD_HTML); }

void handleStatus() {
  char json[224];
  snprintf(json, sizeof(json),
    "{\"fw\":\"%s\",\"temp\":%.2f,\"duty\":%u,\"state\":\"%s\",\"relay\":%s}",
    FW_VERSION, filteredTemp, currentDuty, stateName(state),
    relayOn ? "true" : "false");
  server.send(200, "application/json", json);
}

// ---------------------------------------------------------------------------
// OTA
// ---------------------------------------------------------------------------
void setupOTA() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);

  ArduinoOTA.onStart([]() {
    Serial.println(F("[OTA] Start - entering fail-safe"));
    ledcWrite(PWM_CHANNEL, 0);
    digitalWrite(PIN_RELAY, LOW);
    fill_solid(leds, LED_COUNT, CRGB::Magenta);
    FastLED.show();
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("\n[OTA] End"));
    fill_solid(leds, LED_COUNT, CRGB::White);
    FastLED.show();
  });
  ArduinoOTA.onProgress([](unsigned int p, unsigned int t) {
    Serial.printf("[OTA] %u%%\r", (p * 100) / t);
  });
  ArduinoOTA.onError([](ota_error_t e) {
    Serial.printf("[OTA] Error [%u]\n", e);
    fill_solid(leds, LED_COUNT, CRGB::Red);
    FastLED.show();
  });
  ArduinoOTA.begin();
  Serial.printf("[OTA] Ready @ %s.local\n", OTA_HOSTNAME);
}

// ---------------------------------------------------------------------------
// Setup / Loop
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  printBanner();

  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  analogReadResolution(12);

  setupPWM();
  setupLEDs();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print(F("[WiFi] Connecting"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print('.');
  }
  Serial.printf("\n[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());

  setupOTA();

  udp.begin(UDP_PORT);
  ws.begin();
  ws.onEvent([](uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
    if (type == WStype_CONNECTED)
      Serial.printf("[WS] Client %u connected\n", num);
  });

  server.on("/",       handleRoot);
  server.on("/status", handleStatus);
  server.begin();

  Serial.println(F("[BOOT] Ready"));
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  ws.loop();

  unsigned long now = millis();

  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample   = now;
    currentTemp  = readNTC();
    filteredTemp = (TEMP_FILTER_ALPHA * currentTemp) +
                   ((1 - TEMP_FILTER_ALPHA) * filteredTemp);

    state        = computeState(filteredTemp, state);
    currentDuty  = computeDuty(filteredTemp);
    applyDuty(currentDuty);

    relayOn = (state == CRITICAL);
    digitalWrite(PIN_RELAY, relayOn ? HIGH : LOW);

    updateLEDs(state);
  }

  if (now - lastTelemetry >= TELEMETRY_INTERVAL_MS) {
    lastTelemetry = now;
    sendTelemetry();
  }
}
