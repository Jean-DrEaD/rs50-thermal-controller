#ifndef CONFIG_H
#define CONFIG_H

// =====================================================
// RS50 Thermal Controller - Configuração Global
// =====================================================

// ---------- Firmware Version ----------
// IMPORTANTE: manter no formato "3.3.X" — validado pelo hook .githooks/pre-commit
// e pela linha 2 de rs50_thermal.ino (// RS50 Thermal Controller vX.Y.Z)
#define FW_VERSION        "3.3.9"

// ---------- Wi-Fi ----------
#define WIFI_SSID         "SUA_REDE_WIFI"
#define WIFI_PASS         "SUA_SENHA_WIFI"
#define WIFI_HOSTNAME     "rs50"

// ---------- Portas de rede ----------
#define WEB_PORT          80
#define WS_PORT           81
#define UDP_PORT          33333      // Porta UDP para SimHub

// ---------- Pinos ESP32 ----------
#define PIN_NTC           34         // ADC1_CH6 (entrada analógica)
#define PIN_PWM           25         // Saída PWM para cooler/fan
#define PIN_RELAY         26         // Saída digital relé (NC fail-safe)
#define PIN_SDA           21         // I2C OLED
#define PIN_SCL           22         // I2C OLED

// ---------- PWM ----------
#define PWM_CHANNEL_FAN   0
#define PWM_FREQ          25000      // 25 kHz (silencioso, fora do audível)
#define PWM_RESOLUTION    8          // 8 bits (0-255)
#define PWM_MIN           20         // % mínimo (cooler nunca para)
#define PWM_MAX           100        // % máximo

// ---------- NTC (Steinhart-Hart) ----------
#define ADC_MAX           4095.0f    // ESP32 ADC 12 bits
#define NTC_SERIES_R      10000.0f   // Resistor série 10k
#define NTC_NOMINAL_R     10000.0f   // NTC 10k @ 25°C
#define NTC_NOMINAL_T     298.15f    // 25°C em Kelvin
#define NTC_BETA          3950.0f    // Coeficiente Beta típico
#define THERMAL_OFFSET    0.0f       // Ajuste fino de calibração (°C)

// ---------- Filtragem ----------
#define TEMP_FILTER_ALPHA 0.2f       // EMA: 0=lento, 1=rápido
#define SAMPLE_INTERVAL_MS 500       // Amostragem do NTC
#define TELEMETRY_INTERVAL_MS 1000   // Envio WebSocket/UDP

// ---------- Limiares térmicos (°C) ----------
#define TEMP_IDLE         30.0f      // Abaixo disto: IDLE
#define TEMP_WARMING      45.0f      // Início aquecimento
#define TEMP_WARNING      65.0f      // Atenção
#define TEMP_CRITICAL     80.0f      // Crítico → aciona relé
#define TEMP_MIN          TEMP_IDLE
#define TEMP_MAX          TEMP_CRITICAL
#define THERMAL_HYSTERESIS 2.0f      // Histerese para evitar oscilação

// ---------- Relé ----------
extern bool relayOn;                 // Flag global de estado do relé

// ---------- Helpers ----------
inline const char* stateName(int s) {
  switch (s) {
    case 0: return "IDLE";
    case 1: return "WARMING";
    case 2: return "WARNING";
    case 3: return "CRITICAL";
    default: return "UNKNOWN";
  }
}

#endif // CONFIG_H
