# Changelog

Todas as mudanças notáveis neste projeto serão documentadas aqui.

O formato segue [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/),
e o projeto adere a [Semantic Versioning](https://semver.org/lang/pt-BR/).

## [Unreleased]

### Planejado
- OTA updates via WiFi
- Configuração via dashboard web
- Histórico CSV em SPIFFS
- API REST `/api/status`, `/api/config`
- MQTT integration (Home Assistant)

---

## [3.3.0] - 2026-05-01

### ✨ Adicionado
- Suporte a múltiplos LEDs WS2812 (1, 3, 5+ configurável)
  - **Single mode**: 1 LED com cor por estado
  - **Bar mode**: 3-4 LEDs ([estado][temperatura][fan][pico])
  - **Thermometer mode**: 5+ LEDs em gradiente vertical
- Reconexão automática WiFi a cada 30s
- Indicador online/offline no dashboard
- Pico de temperatura da sessão no dashboard
- Versão do firmware exposta no dashboard

### 🔧 Modificado
- `THERMAL_OFFSET` agora documentado para calibração
- LED em estado NORMAL agora é praticamente invisível (brilho 3/255)
- Estado CRITICAL pulsa a 2Hz (era estrobo rápido)
- FAULT com grace period adaptativo:
  - T<50°C → 60s tolerância
  - T 50-55°C → 30s
  - T>55°C → 5s
- Cor LED corrigida para GRB (Waveshare WS2812B)

### 🐛 Corrigido
- Boot com fan em 100% por 2s para teste antes de habilitar relé
- Pull-down 10kΩ no GPIO do relé previne acionamento durante reset

### 📚 Documentação
- Adicionado `docs/calibration-guide.md`
- Adicionado `docs/wiring-schematic.svg`
- Adicionado `hardware/bom.csv`
- Adicionado workflows GitHub Actions

---

## [3.2.0] - 2026-04-28

### ✨ Adicionado
- Dashboard WiFi com WebSocket em tempo real
- Contador persistente de horas de uso (NVS)
- FAULT inteligente com grace period

### 🔧 Modificado
- Migração para relé fail-safe NC (Songle SLA-24VDC-SL-C)
- LED com brilho ajustável por estado

---

## [3.1.0] - 2026-04-25

### ✨ Adicionado
- Estimativa adaptativa de temperatura do estator
- Lead time para antecipar rampas térmicas
- Detecção de falha do sensor (FAULT)

---

## [3.0.0] - 2026-04-20

### 🎉 Release Inicial
- Controle PWM de fan baseado em NTC
- Histerese para evitar oscilação
- Filtro EMA para reduzir ruído
- Telemetria via Serial

[Unreleased]: https://github.com/Jean-DrEaD/rs50-thermal-controller/compare/v3.3.0...HEAD
[3.3.0]: https://github.com/Jean-DrEaD/rs50-thermal-controller/releases/tag/v3.3.0
[3.2.0]: https://github.com/Jean-DrEaD/rs50-thermal-controller/releases/tag/v3.2.0
[3.1.0]: https://github.com/Jean-DrEaD/rs50-thermal-controller/releases/tag/v3.1.0
[3.0.0]: https://github.com/Jean-DrEaD/rs50-thermal-controller/releases/tag/v3.0.0
