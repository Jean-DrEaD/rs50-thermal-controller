# Changelog

Todas as mudanças relevantes deste projeto são documentadas aqui.
Formato: [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/) · SemVer.

## [3.3.12] - 2026-05-13

### Added
- OTA support (`ArduinoOTA`) with visual fail-safe on WS2812 strip
  (magenta = start, white = done, red = error).
- Boot banner with firmware version, build date/time and chip ID.
- `fw` field included in WebSocket and `/status` JSON payloads.

### Changed
- Relay logic consolidated: active HIGH on GPIO4, engaged **only** in
  `CRITICAL` state (fail-safe forced LOW during OTA).
- LED strip now reflects FSM state in real time (blue/green/yellow/red).
- `FW_VERSION` bumped from `3.3.11` → `3.3.12`.

### Fixed
- Duplicate `setupLEDs()` call removed from `setup()`.
- PWM initial write now starts at 0 instead of `PWM_MIN` to avoid
  fan kick on cold boot.
- FSM hysteresis applied consistently across all transitions.


## [3.3.11] — 2026-05-13

### Added
- `src/rs50_thermal/config.h.example` versionado como template público
  (sem credenciais Wi-Fi, OTA password ou tokens).
- Seção **"Procedimento de Download Manual (GPIO0 + RESET)"** no `README.md`
  para casos em que o auto-reset via USB-CDC falha.
- Script `flash.ps1` (PowerShell) para upload local via `arduino-cli`,
  parametrizado por `-Port COMx`.

### Changed
- **FQBN do build PowerShell** consolidado e validado:
  `esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashMode=qio,FlashSize=4M,PartitionScheme=default,PSRAM=disabled`
  - Removida opção inválida `FlashFreq` que rejeitava o build no `arduino-cli`.
  - `PSRAM=disabled` agora é padrão explícito (S3-Zero **não tem PSRAM** —
    com `opi` compila mas trava no boot esperando RAM externa, mesmo problema
    documentado na v3.3.6 para `release.yml`).
- HTML/JS do dashboard movido de raw literal inline no `.ino` para
  `web_ui.h` separado, seguindo a mesma regra já aplicada ao `dashboard.h`
  na v3.3.8 (delimitador raw ≤ 16 chars).

### Fixed
- **Boot loop pós-upload via USB-CDC**: resolvido documentando o
  procedimento manual (segurar GPIO0 → pulsar RESET → soltar GPIO0 →
  upload → RESET final). O `auto-reset` do S3-Zero é instável quando
  `USBMode=hwcdc` está ativo durante o flash.
- Perda de porta COM durante upload em modo automático — agora documentado
  o fallback manual no README.
- Divergência residual entre `config.h` e `README.md` em macros mencionadas
  como "configuráveis" (reforço da Lição #9 documentada na v3.3.10).

### Removed
- `src/rs50_thermal/config.h` local (com credenciais) removido do tracking Git.
  Adicionado ao `.gitignore`. Usar `config.h.example` como base.

### Notes
- `arduino-cli monitor` não oferece reset/DTR como o Serial Monitor da IDE
  Arduino. Para debug serial, recomendado: **PlatformIO**, **Tera Term**
  ou **PuTTY** (todos respeitam o handshake CDC do S3).
- Topologia do relé permanece **fail-safe** (NC energizado mantém motor
  ativo; falha de MCU ou energia → relé desarma → carga preservada).

### Lessons learned (incorporadas ao CONTEXT.md)
- **#11**: `FlashFreq` não é parâmetro válido do FQBN para `esp32s3` —
  só `esp32` clássico aceita. Verificar com `arduino-cli board details`
  antes de adicionar opções ao FQBN.
- **#12**: USB-CDC nativo do S3 + `auto-reset` é frágil — sempre ter
  fallback manual documentado.

---

## [3.3.10] — 2026-05-13

### Added
- Macro `LED_COUNT` (5) em `config.h` — antes mencionada no README mas ausente.
- Macro `PIN_WS2812` (GPIO 9) e `LED_BRIGHTNESS` (64).
- Macro `TEMP_RESTART` (63°C) para histerese pós-shutdown explícita.
- Lições aprendidas #8, #9 e #10 no `CONTEXT.md`.

### Changed
- **Pinout migrado para ESP32-S3-Zero** (estava com pinos do ESP32 clássico):
  - `PIN_NTC`: 34 → 1
  - `PIN_RELAY`: 26 → 4
  - `PIN_PWM`: 25 → 5
- **NTC**: 10k → 100k B3950 (alinhado ao sensor real no estator).
- **Thresholds térmicos** alinhados ao `CONTEXT.md`:
  - `TEMP_IDLE`: 30 → 40°C
  - `TEMP_WARNING`: 65 → 60°C
  - `TEMP_CRITICAL`: 80 → 68°C
  - `THERMAL_HYSTERESIS`: 2.0 → 5.0°C
- `PWM_MIN`: 20 → 0 (fan desliga em repouso, menos sujeira).
- Comentário do pinout do relé esclarecido (HIGH = aciona MOSFET = abre NC).

### Removed
- `#define UDP_PORT 33339` duplicado no `.ino` (já vinha do `config.h` como 33333).
- `#include <Wire.h>`, `Wire.begin()`, `PIN_SDA`, `PIN_SCL` (I2C não usado em v3.x).
- Função `stateName(int)` inline em `config.h` (dead code — `.ino` tem versão com `ThermalState`).
- Placeholder `<owner>` no header do `.ino` (corrigido para `Jean-DrEaD`).

### Fixed
- Comentário `R_SERIES -> 3V3` corrigido para `NTC_SERIES_R -> 3V3` (nome real da macro).
- Trailing whitespace no `.ino`.

---

## [3.3.9] — 2026-05-12

### Fixed

- 🚨 **Include órfão `Adafruit_GFX.h`**: a linha `//#include <Adafruit_GFX.h>`
  estava comentada em `src/rs50_thermal/rs50_thermal.ino` (linha 12). O CI do
  workflow de release tentava resolver a dependência mesmo comentada, quebrando
  o build. Linha **removida por completo** — o projeto não usa display gráfico e
  não há roadmap para `Adafruit_GFX`.
  > 💡 **Lição**: comentar includes não os torna inertes em toda toolchain.
  > Remover é mais seguro que comentar.

### Process

- Tag `v3.3.9` foi apagada (local e remota) após CI falhar, corrigida em branch
  `hotfix/v3.3.9-missing-include` e recriada após merge com CI verde.

---

## [3.3.8] — 2026-05-07

### Added

- 📊 **`dashboard.h`** extraído do `.ino`: HTML/CSS/JS do dashboard web agora
  em arquivo separado, evitando que template literals JS (`${}`) confundam o
  parser do `arduino-cli`.
- 📦 **`ci/libraries.txt`**: libs e versões centralizadas em um único arquivo
  (uma por linha, formato `Nome@versão`), instaladas em loop pelo workflow.
  Garante reprodutibilidade entre máquina local e CI.

### Fixed

- **Raw string delimiter de 19 chars** (`DASHBOARD_RS50_V338`) causava
  `error: raw string delimiter longer than 16 characters`. Substituído por
  `RS50DASH338` (11 chars).
- **Template literals JS** (`${...}` + arrow functions) dentro do raw literal
  C++ causavam erros de parse fantasma. Convertidos para concatenação `+` e
  `function() {...}`.

### Changed

- Workflows `build.yml` e `release.yml`: libs agora lidas de `ci/libraries.txt`
  ao invés de instaladas individualmente em cada step.

---

## [3.3.7] — 2026-05-06

### Fixed

- 🚨 **Release `.bin` errado**: o workflow `release.yml` da v3.3.6
publicou o `bootloader.bin` (~14 KB) ao invés do firmware principal
(~756 KB). Causa: `find -name "*.bin" | head -1` pegava o primeiro
na ordem alfabética (bootloader vem antes de `ino.bin`). Solução:
filtrar especificamente por `*.ino.bin`.
- **Release agora publica os 3 binários separadamente**: app
(`*.bin`), bootloader (`*_bootloader.bin`) e partições
(`*_partitions.bin`), com instruções de gravação no endereço correto
(`0x10000` para o app, não `0x0`).

### Added

- 📈 **Gráfico histórico no dashboard** (Chart.js): linha temporal
dos últimos 5 minutos com 3 séries (Eixo, Estator, Fan PWM).
Eixos duplos (temperatura à esquerda, fan % à direita).
- 🪝 **Pre-commit hook** em `.githooks/pre-commit`:
  * Valida `FW_VERSION` no padrão `3.3.x`
  * Confere coerência entre header do `.ino` e `FW_VERSION` em `config.h`
  * Detecta macros referenciadas no `.ino` mas ausentes do `config.h`
  * Avisa sobre tabs e trailing whitespace
  * Ativar com: `git config core.hooksPath .githooks`
- 📘 **Documentação de branch protection** no README (passo a passo
manual no GitHub Settings).

### Changed

- Release notes agora documentam **3 modos de gravação**: só app
(atualização), flash completo (recovery) e Web Flasher (mais fácil).

---

## [3.3.6] — 2026-05-06

### Fixed

- **CI workflow `release.yml`**: corrigido step "Rename artifacts" que
falhava com `mv: target 'rs50_thermal_v3.3.5.bin': No such file`.
  * Causa: `arduino-cli` salva binários em subpasta
`./release/rs50_thermal/`, não direto em `./release/`.
  * Solução: usar `find ./release -name "*.bin"` para localizar.
- **CI workflow `arduino-build.yml`**: estava usando ESP32 core
**3.0.7** + FastLED 3.7.8 + WebSockets 2.7.2, incompatíveis com a API
`ledcSetup`/`ledcAttachPin` usada no firmware. Migrado para core
**2.0.14** + FastLED 3.6.0 + WebSockets 2.4.1 (alinhado com release.yml
e ambiente local).

### Changed

- **`release.yml`**: FQBN agora usa `PSRAM=disabled` (era `PSRAM=opi`).
O ESP32-S3-Zero da Waveshare **não tem PSRAM** — com `opi` o firmware
compila mas trava no boot esperando RAM externa.
- **`release.yml`**: `setup-arduino-cli@v1` → `@v2`.
- **`release.yml`**: `softprops/action-gh-release@v1` → `@v2`.
- **`arduino-build.yml`**: lint de tabs/whitespace agora gera warning
(era erro fatal que bloqueava merge).

---

## [3.3.5] — 2026-05-06

### Fixed

- **`config.h` faltando `PWM_CHANNEL_FAN`**: o `.ino` da v3.3.4 referenciava
`PWM_CHANNEL_FAN` mas o header só definia `PWM_CHANNEL`. Build do CI
falhava com `'PWM_CHANNEL_FAN' was not declared`. Renomeado o define
para o nome usado no firmware.
- **Versão dessincronizada no `config.h`**:
  * Comentário do header dizia `Version: 3.3.0 (FINAL)` → atualizado para `3.3.5`.
  * `FW_VERSION` estava em `"3.3.2"` (defasado) → atualizado para `"3.3.5"`.
- **`README.md`**: seção "Estrutura do repositório" mencionava
`docs/wiring-schematic-v3.3.1.svg` (deletado) e diretórios inexistentes
(`firmware/`, `platformio.ini`). Atualizada para refletir a estrutura
real do repo.

### Changed

- `.ino` header bumpado para `v3.3.5`.

---

## [3.3.4] — 2026-05-06

### Fixed

- **Patches do `.ino` aplicados de fato** (a v3.3.3 documentou mas não
modificou o código). Confirmadas as 4 ocorrências com nova API:
  * `ledcSetup()` + `ledcAttachPin()` no setup
  * 3× `ledcWrite(PWM_CHANNEL_FAN, ...)` em vez de `PIN_PWM`
- **Comentários enganosos**: removidas referências a "Core 3.x" no
header e na seção de ADC. Agora declaram corretamente Core 2.0.x.
- **`getStateColor()` e `getStateBrightness()`**: adicionado `default` em todos os switches (elimina warnings do compilador).
- **`printTelemetry()`**: variáveis `state` e `icon` agora inicializadas
(defesa contra UB se enum crescer no futuro).
- **`renderLEDs_Bar()`**: adicionados guards `if (LED_COUNT >= N)` antes
de acessar `leds[1]` e `leds[2]`.
- **Footer do dashboard HTML**: agora usa versão dinâmica via WebSocket
em vez de string hardcoded `v3.3.0`.
- **README**: link da imagem corrigido (apontava para SVG deletado).

### Changed

- `docs/wiring-schematic.svg`: substituído pelo **export oficial do
Fritzing** (~1.87 MB, breadboard view fiel à montagem física).

### Removed

- `docs/wiring-schematic-v3.3.1.svg` (substituído pelo Fritzing).

---

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

---

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

- **Workflow CI/Compile firmware**: erro `'ledcAttach' was not declared` corrigido. Substituído `ledcAttach(pin, freq, res)` (API v3.x) por
`ledcSetup(ch, freq, res)` + `ledcAttachPin(pin, ch)` (API v2.x),
compatível com `esp32:esp32@2.0.14` usado no workflow.

---

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

---

## [3.3.0] — 2026-05-05

### Added

- Topologia **fail-safe** com relé Songle SLA-24VDC-SL-C em **NC** (corta apenas +24V para a MKS; GND vai direto da fonte).
- Driver do relé com IRLZ44N + 220Ω no gate + pull-down 10k + 1N4007.
- Divisor NTC 100k B3950 com cap 100nF para GPIO1.
- Fan 120mm 12V PWM nativo controlado por GPIO5 (3.3V direto).
- WS2812 status no GPIO9 com 330Ω série.

### Changed

- Migração do esquemático para SVG inline (descartado o Fritzing
como fonte da verdade — apenas referência de breadboard).

---

## [3.2.0] — 2026-05-03

- Cadeia de reguladores HW-411 (LM2596): 24V → 12V → 5V.
- Mudança de DRV8871 para topologia de relé pelo critério fail-safe.

---

## [3.1.0] — 2026-05-01

- Primeiro protótipo com ESP32-S3-Zero + NTC + cooler.

