# Changelog

Todas as mudanças relevantes deste projeto são documentadas aqui.
Formato: [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/) · SemVer.

## [3.3.3] — 2026-05-06

### Fixed
- **Firmware compila no CI**: substituído `ledcAttach()` (API ESP32 core 3.x)
  por `ledcSetup()` + `ledcAttachPin()` (API core 2.x), compatível com
  `esp32:esp32@2.0.14` usado no workflow.
- Todas as chamadas `ledcWrite(PIN_PWM, ...)` migradas para
  `ledcWrite(PWM_CHANNEL_FAN, ...)` conforme exigido pela API v2.x.
- Adicionadas constantes `PWM_CHANNEL_FAN = 0`, `PWM_FREQ = 25000`,
  `PWM_RESOLUTION = 8`.

### Changed
- Comentários do header do `.ino` atualizados para refletir core 2.0.14.

### Restored
- `CONTEXT.md` reintroduzido em versão **resumida** (uma página).

### Removed
- (mantém o que foi removido na 3.3.2: scripts locais).

## [3.3.2] — 2026-05-06

### Changed
- **Schematic substituído pelo diagrama Fritzing** (`docs/wiring-diagram.svg`).
  O SVG sintético gerado anteriormente foi descartado em favor do export
  fiel ao breadboard real.

### Removed
- `docs/wiring-schematic-v3.3.1.svg` (substituído).
- `CONTEXT.md` (consolidado dentro do README).
- `setup-repo.ps1` e `update-repo.ps1` (scripts locais, não versionar).

### Fixed
- **Workflow CI/Compile firmware**: erro `'ledcAttach' was not declared`
  corrigido. Substituído `ledcAttach(pin, freq, res)` (API v3.x) por
  `ledcSetup(ch, freq, res)` + `ledcAttachPin(pin, ch)` (API v2.x),
  compatível com `esp32:esp32@2.0.14` usado no workflow.

## [3.3.1] — 2026-05-06

### Fixed
- **Polaridade do diodo flyback 1N4007**: documentação visual corrigida e validada.
  Cátodo (faixa) no lado **+24V/B1/COM**, anodo no lado **Drain/B2**.
  (O diodo físico já estava correto desde o início; apenas a leitura do
  schematic em revisão estava ambígua.)
- Labels `Anodo` / `Katodo` adicionados ao Fritzing para evitar futura
  confusão visual.
- Labels `B1=VCC` / `B2=VAI PRO DRAIN` no relé Songle para deixar
  explícito o sentido da bobina.

### Added
- Novo `docs/wiring-schematic-v3.3.1.svg` (versão definitiva, renderiza
  nativo no GitHub).
- Seção **"Lógica fail-safe"** dentro do próprio SVG.
- Seção **"Como funciona o flyback diode"** no README.
- Mnemônico **"K = Katodo = faixa"** documentado no README como
  aprendizado do projeto.

### Changed
- `README.md` reescrito com fluxo de operação em 3 estados
  (repouso / shutdown / desligamento do MOSFET).
- `CONTEXT.md` atualizado com decisões de hardware fechadas.

## [3.3.0] — 2026-05-05

### Added
- Topologia **fail-safe** com relé Songle SLA-24VDC-SL-C em **NC**
  (corta apenas +24V para a MKS; GND vai direto da fonte).
- Driver do relé com IRLZ44N + 220Ω no gate + pull-down 10k + 1N4007.
- Divisor NTC 100k B3950 com cap 100nF para GPIO1.
- Fan 120mm 12V PWM nativo controlado por GPIO5 (3.3V direto).
- WS2812 status no GPIO9 com 330Ω série.

### Changed
- Migração do esquemático para SVG inline (descartado o Fritzing
  como fonte da verdade — apenas referência de breadboard).

## [3.2.0] — 2026-05-03
- Cadeia de reguladores HW-411 (LM2596): 24V → 12V → 5V.
- Mudança de DRV8871 para topologia de relé pelo critério fail-safe.

## [3.1.0] — 2026-05-01
- Primeiro protótipo com ESP32-S3-Zero + NTC + cooler.
