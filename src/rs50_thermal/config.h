// ════════════════════════════════════════════════════════════════════════════
//  RS50 Thermal Controller — config.h
//  Version: 3.3.0 (FINAL)
//  Author: DrEaD (Joinville/SC)
//  Hardware: ESP32-S3-Zero Waveshare + NTC 100K B3950 + Songle SLA-24VDC-SL-C
//  Target: Volante DD RS50 Clone + Hoverboard 15Nm + ODESC FFBeast
// ════════════════════════════════════════════════════════════════════════════

#ifndef CONFIG_H
#define CONFIG_H

#define FW_VERSION "3.3.0"
#define FW_NAME    "RS50-Thermal"

// ═══════════════════════════════════════════════════════════════════════════
//  PINOS — ESP32-S3-Zero Waveshare
// ═══════════════════════════════════════════════════════════════════════════
#define PIN_PWM         5     // Fan PWM (4-pin, sinal azul)
#define PIN_NTC         1     // ADC1_CH0 — NTC no eixo
#define PIN_RELAY       4     // Driver MOSFET do relé
#define PIN_LED_RGB     21    // WS2812 onboard (e/ou strip externa)

// ═══════════════════════════════════════════════════════════════════════════
//  LEDs ENDEREÇÁVEIS — escolha quantos vai usar
// ═══════════════════════════════════════════════════════════════════════════
//  1 LED  → onboard apenas (estado simples por cor)
//  3 LEDs → barra: [estado] [temperatura] [fan]
//  5 LEDs → termômetro vertical (gradiente verde→vermelho)
//  N LEDs → modo termômetro estendido
// ═══════════════════════════════════════════════════════════════════════════
#define LED_COUNT          1            // ⚙️ AJUSTE AQUI: 1, 3, 5 ou mais
#define LED_COLOR_ORDER    GRB           // Waveshare = GRB!
#define LED_TYPE           WS2812B
#define LED_BRIGHTNESS_MAX 80
#define LED_BRIGHTNESS_NORMAL 3          // Quase invisível em NORMAL
#define LED_BRIGHTNESS_INFO   25

// Modos automáticos (não mexer):
#define LED_MODE_SINGLE      (LED_COUNT == 1)
#define LED_MODE_BAR         (LED_COUNT >= 3 && LED_COUNT <= 4)
#define LED_MODE_THERMOMETER (LED_COUNT >= 5)

// ═══════════════════════════════════════════════════════════════════════════
//  NTC HT-NTC100K B3950 (testado: 100kΩ @ 24°C ✅)
// ═══════════════════════════════════════════════════════════════════════════
#define R_FIXED         100000.0f
#define NTC_BETA        3950.0f
#define R_NTC_25C       100000.0f
#define T_NOMINAL_K     298.15f
#define VCC_ADC         3.3f
#define ADC_RES         4095.0f

// ═══════════════════════════════════════════════════════════════════════════
//  COMPENSAÇÃO TÉRMICA (eixo → estator)
// ═══════════════════════════════════════════════════════════════════════════
#define THERMAL_OFFSET    9.0f    // ⚠️ CALIBRAR após 1ª sessão (ver guia)
#define LEAD_TIME_S       70.0f   // Antecipação em rampas térmicas
#define DTDT_TRIGGER      0.04f   // °C/s mínimo para ativar lead time

// ═══════════════════════════════════════════════════════════════════════════
//  CURVA PWM DA FAN (perfil "casual + conforto térmico")
// ═══════════════════════════════════════════════════════════════════════════
#define TEMP_MIN          30.0f   // Liga discreto (proteger ímãs N35)
#define TEMP_MAX          52.0f   // Fan 100% antes da zona de risco
#define PWM_MIN           40      // % mínimo (sempre tem fluxo de ar)
#define PWM_MAX           100

// ═══════════════════════════════════════════════════════════════════════════
//  HISTERESE & FILTROS
// ═══════════════════════════════════════════════════════════════════════════
#define TEMP_HYSTERESIS   1.5f    // Evita oscilação de PWM
#define EMA_ALPHA         0.18f   // Filtro exponencial (0.1-0.3)
#define DTDT_WINDOW_MS    30000   // Janela para cálculo de derivada

// ═══════════════════════════════════════════════════════════════════════════
//  PROTEÇÃO MULTI-NÍVEL (limites por T_estator estimada)
// ═══════════════════════════════════════════════════════════════════════════
#define TEMP_WARNING      55.0f
#define TEMP_CRITICAL     62.0f
#define TEMP_SHUTDOWN     68.0f   // ACIONA RELÉ — corta 24V
#define TEMP_RECOVERY     50.0f   // Religa só após esfriar
#define ADC_FAULT_LO      50      // Detecção curto/aberto
#define ADC_FAULT_HI      4045

// ═══════════════════════════════════════════════════════════════════════════
//  RELÉ Songle SLA-24VDC-SL-C (SPDT, bobina 24V)
// ═══════════════════════════════════════════════════════════════════════════
//  Lógica fail-safe (NC):
//    GPIO 4 LOW  = MOSFET OFF = bobina desenergizada = NC fechado = motor ON
//    GPIO 4 HIGH = MOSFET ON  = bobina energizada    = NC aberto  = motor OFF
//  Se ESP32 travar/resetar (GPIO floating com pull-down 10k) → motor LIGADO
// ═══════════════════════════════════════════════════════════════════════════
#define RELAY_FAILSAFE_NC   true
#define SHUTDOWN_LATCH_MS   60000  // Mínimo 60s desligado

// ═══════════════════════════════════════════════════════════════════════════
//  FAULT ADAPTATIVO (grace period varia com a temperatura)
// ═══════════════════════════════════════════════════════════════════════════
#define FAULT_GRACE_LOW     60000  // T<50°C → 60s tolerância
#define FAULT_GRACE_MID     30000  // T 50-55°C → 30s
#define FAULT_GRACE_HIGH    5000   // T>55°C → 5s

// ═══════════════════════════════════════════════════════════════════════════
//  CONTADOR DE HORAS (NVS — persiste após reset)
// ═══════════════════════════════════════════════════════════════════════════
#define HOUR_COUNTER_SAVE_INTERVAL 300000  // Salva a cada 5min
#define ACTIVE_THRESHOLD_TEMP      35.0f   // T mín. para "uso ativo"

// ═══════════════════════════════════════════════════════════════════════════
//  TIMINGS LED (refinados conforme feedback)
// ═══════════════════════════════════════════════════════════════════════════
#define CRITICAL_BLINK_MS   250    // 2Hz
#define SHUTDOWN_STROBE_MS  80     // ~12Hz
#define FAULT_STROBE_MS     200

// ═══════════════════════════════════════════════════════════════════════════
//  AMOSTRAGEM
// ═══════════════════════════════════════════════════════════════════════════
#define SAMPLES           25
#define UPDATE_INTERVAL   1500
#define SERIAL_BAUD       115200

// ═══════════════════════════════════════════════════════════════════════════
//  PWM HARDWARE
// ═══════════════════════════════════════════════════════════════════════════
#define PWM_FREQ          25000    // Acima do audível
#define PWM_RESOLUTION    8
#define PWM_CHANNEL       0

// ═══════════════════════════════════════════════════════════════════════════
//  WIFI / DASHBOARD
// ═══════════════════════════════════════════════════════════════════════════
#define ENABLE_WIFI         true
#define WIFI_SSID           "SuaRedeAqui"     // ⚙️ AJUSTAR
#define WIFI_PASS           "SuaSenhaAqui"    // ⚙️ AJUSTAR
#define WIFI_HOSTNAME       "rs50-thermal"
#define WEB_PORT            80
#define WS_PORT             81
#define WS_UPDATE_MS        1000
#define WIFI_RETRY_MS       30000   // Reconnect automático

#endif // CONFIG_H
