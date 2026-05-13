//
// RS50 Thermal Controller v3.3.11
//  Firmware ESP32 — NTC + PWM + Relé fail-safe + Dashboard WS + UDP SimHub
//  © 2026 — github.com/<owner>/rs50-thermal-controller
//
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <math.h>

#include "config.h"
#include "dashboard.h"

// Broadcast da sua rede local (ajuste conforme sua sub-rede)
IPAddress UDP_BROADCAST_IP(255, 255, 255, 255);

// ---------- Rede ----------
WiFiUDP udp;
WebServer server(80);
WebSocketsServer ws(81);

// ---------- Estado térmico ----------
enum ThermalState { IDLE, WARMING, WARNING, CRITICAL };
ThermalState state = IDLE;
const char* stateName(ThermalState s) {
  switch (s) {
    case IDLE:     return "IDLE";
    case WARMING:  return "WARMING";
    case WARNING:  return "WARNING";
    case CRITICAL: return "CRITICAL";
  }
  return "UNKNOWN";
}

// ---------- Variáveis globais ----------
float currentTemp = 25.0f;
float filteredTemp = 25.0f;
uint8_t currentDuty = PWM_MIN;
bool relayOn = false;
unsigned long lastTelemetry = 0;
unsigned long lastSample = 0;

// ---------- Leitura NTC com Steinhart-Hart ----------
float readNTC() {
  const int samples = 8;
  uint32_t acc = 0;
  for (int i = 0; i < samples; i++) {
    acc += analogRead(PIN_NTC);
    delayMicroseconds(200);
  }
  float adc = acc / (float)samples;
  if (adc <= 0) adc = 1;

  // Divisor: NTC -> GND, NTC_SERIES_R -> 3V3
  float r = NTC_SERIES_R * (adc / (ADC_MAX - adc));
  float lnR = logf(r / NTC_NOMINAL_R);
  float invT = (1.0f / NTC_NOMINAL_T) + (lnR / NTC_BETA);
  float tempK = 1.0f / invT;
  float tempC = tempK - 273.15f + THERMAL_OFFSET;
  return tempC;
}

// ---------- Curva PWM linear entre PWM_MIN e PWM_MAX ----------
uint8_t computeDuty(float temp) {
  if (temp <= TEMP_IDLE)     return PWM_MIN;
  if (temp >= TEMP_CRITICAL) return PWM_MAX;
  float t = (temp - TEMP_IDLE) / (TEMP_CRITICAL - TEMP_IDLE);
  return (uint8_t)(PWM_MIN + t * (PWM_MAX - PWM_MIN));
}

// ---------- Máquina de estados com histerese ----------
ThermalState computeState(float temp, ThermalState prev) {
  const float H = THERMAL_HYSTERESIS;
  switch (prev) {
    case IDLE:
      if (temp > TEMP_WARMING + H) return WARMING;
      break;
    case WARMING:
      if (temp > TEMP_WARNING + H)  return WARNING;
      if (temp < TEMP_WARMING - H)  return IDLE;
      break;
    case WARNING:
      if (temp > TEMP_CRITICAL + H) return CRITICAL;
      if (temp < TEMP_WARNING - H)  return WARMING;
      break;
    case CRITICAL:
      if (temp < TEMP_CRITICAL - H) return WARNING;
      break;
  }
  return prev;
}

// ---------- PWM ----------
void setupPWM() {
  ledcSetup(PWM_CHANNEL_FAN, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PIN_PWM, PWM_CHANNEL_FAN);
  ledcWrite(PWM_CHANNEL_FAN, PWM_MIN);
}

void applyDuty(uint8_t duty) {
   if (duty == 0) { ledcWrite(PWM_CHANNEL_FAN, 0); return; }
  uint32_t maxVal = (1 << PWM_RESOLUTION) - 1;
  uint32_t raw = (duty * maxVal) / 100;
  ledcWrite(PWM_CHANNEL_FAN, raw);
}

// ---------- Telemetria ----------
void sendTelemetry() {
  // JSON via WebSocket
  char json[192];
  snprintf(json, sizeof(json),
    "{\"temp\":%.2f,\"duty\":%u,\"state\":\"%s\",\"relay\":%s,\"uptime\":%lu}",
    filteredTemp, currentDuty, stateName(state),
    relayOn ? "true" : "false", millis() / 1000);
  ws.broadcastTXT(json);

  // CSV via UDP broadcast
  char csv[96];
  snprintf(csv, sizeof(csv), "%.2f,%u,%s,%d,%lu\n",
    filteredTemp, currentDuty, stateName(state),
    relayOn ? 1 : 0, millis() / 1000);
  udp.beginPacket(UDP_BROADCAST_IP, UDP_PORT);
  udp.print(csv);
  udp.endPacket();
}

// ---------- HTTP ----------
void handleRoot() {
  server.send_P(200, "text/html", DASHBOARD_HTML);
}

void handleStatus() {
  char json[192];
  snprintf(json, sizeof(json),
    "{\"temp\":%.2f,\"duty\":%u,\"state\":\"%s\",\"relay\":%s}",
    filteredTemp, currentDuty, stateName(state),
    relayOn ? "true" : "false");
  server.send(200, "application/json", json);
}

// ---------- WebSocket ----------
void onWsEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
  if (type == WStype_CONNECTED) {
    Serial.printf("[WS] Client %u connected\n", num);
  }
}

// ---------- WiFi ----------
void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("Falha WiFi — seguindo offline");
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  analogReadResolution(12);


  setupPWM();
  setupWiFi();

  udp.begin(UDP_PORT);
  ws.begin();
  ws.onEvent(onWsEvent);

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.begin();

  Serial.println("Sistema iniciado.");
}

// ---------- Loop ----------
void loop() {
  server.handleClient();
  ws.loop();

  unsigned long now = millis();

  // Amostragem do sensor
  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = now;
    currentTemp = readNTC();
    // Filtro EMA simples
    filteredTemp = (TEMP_FILTER_ALPHA * currentTemp) +
                   ((1.0f - TEMP_FILTER_ALPHA) * filteredTemp);

    state = computeState(filteredTemp, state);
    currentDuty = computeDuty(filteredTemp);
    applyDuty(currentDuty);

    relayOn = (state == CRITICAL);
    digitalWrite(PIN_RELAY, relayOn ? HIGH : LOW);

  }

  // Telemetria periódica
  if (now - lastTelemetry >= TELEMETRY_INTERVAL_MS) {
    lastTelemetry = now;
    sendTelemetry();
  }
}
