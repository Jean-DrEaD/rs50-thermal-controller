# Changelog

Todas as mudancas notaveis neste projeto serao documentadas aqui.

O formato segue [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/),
e o projeto adere a [Semantic Versioning](https://semver.org/lang/pt-BR/).

## [Unreleased]

### Planejado
- OTA updates via WiFi
- Configuracao via dashboard web
- Historico CSV em SPIFFS
- API REST `/api/status`, `/api/config`
- MQTT integration (Home Assistant)

---

## [3.4.0] - 2026-05-05

### Mudanca de Ferramenta de Design
- **BREAKING (docs)**: Hardware design migrado de KiCad para **Fritzing**
- Arquivos KiCad arquivados em `hardware/legacy/` (preservados no Git)
- Ver [ADR-001](docs/decisions/ADR-001-abandona-kicad.md)

### Adicionado
- `CONTEXT.md` na raiz: briefing completo para LLMs e novos colaboradores
- `docs/decisions/ADR-001-abandona-kicad.md`: registro arquitetural da decisao
- `.gitattributes`: normalizacao de line endings (LF para texto, binary para .fzz)

### Hardware Refinado
- **Fan**: especificado como **120mm 4-pin PWM** (era 80mm generico)
- **Buck converter**: explicitado modulo **LM2596 HW-411** (era LM2596 discreto)
- **Conector de entrada**: padronizado **XT60 femea PCB-mount**
  - Cabo da fonte 24V termina em XT60 macho (anti-curto)
- Arquitetura cascata: 24V -> HW-411 #1 -> 12V -> HW-411 #2 -> 5V
- TACH do fan (cabo verde) documentado como NAO UTILIZADO

### Documentacao
- README atualizado para v3.4 refletindo hardware definitivo
- BOM atualizada com modelos especificos (HW-411, XT60)
- Diagrama de arquitetura no README atualizado
- Pinout do ESP32-S3-Zero consolidado (GPIO21 = LED, era GPIO3 em rascunhos)

### Limpeza Interna
- Removido submodulo Git acidental (`hardware/.history`)
- Backups KiCad (`*.kicad_sch.backup*`) fora do tracking
- Scripts PowerShell de geracao de schematic descontinuados
- `.gitignore` reforcado: KiCad, Arduino IDE (`*.ino.txt`), secrets, OS files
- Removida duplicata `rs50_thermal.ino.txt` (hash MD5 confirmou identidade)

---

## [3.3.0] - 2026-05-01

### Adicionado
- Suporte a multiplos LEDs WS2812 (1, 3, 5+ configuravel)
  - **Single mode**: 1 LED com cor por estado
  - **Bar mode**: 3-4 LEDs ([estado][temperatura][fan][pico])
  - **Thermometer mode**: 5+ LEDs em gradiente vertical
- Reconexao automatica WiFi a cada 30s
- Indicador online/offline no dashboard
- Pico de temperatura da sessao no dashboard
- Versao do firmware exposta no dashboard

### Modificado
- `THERMAL_OFFSET` agora documentado para calibracao
- LED em estado NORMAL agora e praticamente invisivel (brilho 3/255)
- Estado CRITICAL pulsa a 2Hz (era estrobo rapido)
- FAULT com grace period adaptativo:
  - T<50C -> 60s tolerancia
  - T 50-55C -> 30s
  - T>55C -> 5s
- Cor LED corrigida para GRB (Waveshare WS2812B)

### Corrigido
- Boot com fan em 100% por 2s para teste antes de habilitar rele
- Pull-down 10K no GPIO do rele previne acionamento durante reset

### Documentacao
- Adicionado `docs/calibration-guide.md`
- Adicionado `docs/wiring-schematic.svg`
- Adicionado `hardware/bom.csv`
- Adicionado workflows GitHub Actions

---

## [3.2.0] - 2026-04-28

### Adicionado
- Dashboard WiFi com WebSocket em tempo real
- Contador persistente de horas de uso (NVS)
- FAULT inteligente com grace period

### Modificado
- Migracao para rele fail-safe NC (Songle SLA-24VDC-SL-C)
- LED com brilho ajustavel por estado

---

## [3.1.0] - 2026-04-25

### Adicionado
- Estimativa adaptativa de temperatura do estator
- Lead time para antecipar rampas termicas
- Deteccao de falha do sensor (FAULT)

---

## [3.0.0] - 2026-04-20

### Release Inicial
- Controle PWM de fan baseado em NTC
- Histerese para evitar oscilacao
- Filtro EMA para reduzir ruido
- Telemetria via Serial

[Unreleased]: https://github.com/Jean-DrEaD/rs50-thermal-controller/compare/v3.4.0...HEAD
[3.4.0]: https://github.com/Jean-DrEaD/rs50-thermal-controller/compare/v3.3.0...v3.4.0
[3.3.0]: https://github.com/Jean-DrEaD/rs50-thermal-controller/releases/tag/v3.3.0
[3.2.0]: https://github.com/Jean-DrEaD/rs50-thermal-controller/releases/tag/v3.2.0
[3.1.0]: https://github.com/Jean-DrEaD/rs50-thermal-controller/releases/tag/v3.1.0
[3.0.0]: https://github.com/Jean-DrEaD/rs50-thermal-controller/releases/tag/v3.0.0
