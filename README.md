# RS50 Thermal Controller v3.3.0

Sistema de proteção e controle térmico para volante DD com motor Hoverboard 15Nm
(stator 50×30mm) acionado por ODESC FFBeast.

## ✨ Features

- 🌡️ Monitoramento térmico em tempo real via NTC no eixo
- 🧠 Estimativa adaptativa da temperatura do estator
- 🌀 Controle PWM de fan com curva otimizada
- ⚡ Proteção via relé fail-safe (sem energia = motor ligado)
- 💡 LEDs WS2812 configuráveis (1, 3, 5+)
- 📊 Dashboard WiFi com WebSocket em tempo real
- ⏱️ Contador persistente de horas de uso
- 🔄 Reconexão automática WiFi
- 🚨 FAULT inteligente com grace period adaptativo

## 🛠️ Hardware

| Componente | Modelo |
|---|---|
| MCU | ESP32-S3-Zero (Waveshare) |
| Sensor | NTC 100K B3950 |
| Relé | Songle SLA-24VDC-SL-C |
| Driver | MOSFET IRLZ44N |
| Fan | 120mm 4-pin PWM 12V |
| Step-downs | 2× LM2596 (24→5V e 24→12V) |

## 📦 Bibliotecas

