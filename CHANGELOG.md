# Changelog

Todas as mudanças relevantes deste projeto são documentadas aqui.
Formato: [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/) · SemVer.

## [3.3.9] — 2026-05-12

### Added
- 📡 **Telemetria SimHub via UDP broadcast** (`255.255.255.255:4242`):
  ESP32 envia o frame `$SH;...` pra rede inteira, SimHub escuta na
  mesma porta. **Zero configuração de IP** em ambos os lados.
- **Fallback automático**: WiFi online → UDP broadcast | WiFi offline
  → Serial (cabo USB / debug). Frame montado uma vez só com `snprintf`
  e reutilizado pelos dois caminhos — sem duplicação.
- Novas constantes em `config.h`: `SIMHUB_UDP_PORT = 4242`,
  `ENABLE_SIMHUB`, `SIMHUB_OUTPUT_MS = 100` (10Hz).
- Includes: `<WiFiUDP.h>` + objeto global `udpBroadcast`.
- `setupWiFi()` agora inicializa também `udpBroadcast.begin(SIMHUB_UDP_PORT)`
  junto com o `server.begin()` e `webSocket.begin()`.

### Changed
- `sendSimHubTelemetry()` reescrito para usar `snprintf` num buffer
  único de 128 bytes, com fallback automático Serial↔UDP.
- Frame SimHub estendido: agora inclui `dtdt`, `peak` e `hours` além
  dos campos originais (`shaft`, `stator`, `fan`, `state`, `motor`).

### Notes
- ESP32 e PC com SimHub precisam estar na **mesma rede LAN**
  (broadcast não atravessa roteadores — comportamento desejado).
- No SimHub: `Devices → Add device → Custom UDP`, porta `4242`,
  delimiter `\n`, mesmo parsing `$SH;key=value;` do Custom Serial.


## [3.3.8] — 2026-05-07

### Fixed
- Hotfix CI - move HTML para dashboard.h separado
- Move HTML_PAGE inteiro do .ino para src/rs50_thermal/dashboard.h
- Usa delimitador unico DASHBOARD_RS50_V338 (em vez de rawliteral)
- Reescreve JS removendo template literals e arrow functions
- Usa concatenacao de strings e function() para maxima compatibilidade
  com o parser do arduino-cli / ESP32 Core 2.0.14
- Bump FW_VERSION 3.3.7 -> 3.3.8
- Resolve erro: 'function' does not name a type (linha 496)
- delimitador raw string respeitando limite de 16 chars

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
  - Valida `FW_VERSION` no padrão `3.3.x`
  - Confere coerência entre header do `.ino` e `FW_VERSION` em `config.h`
  - Detecta macros referenciadas no `.ino` mas ausentes do `config.h`
  - Avisa sobre tabs e trailing whitespace
  - Ativar com: `git config core.hooksPath .githooks`
- 📘 **Documentação de branch protection** no README (passo a passo
  manual no GitHub Settings).

### Changed
- Release notes agora documentam **3 modos de gravação**: só app
  (atualização), flash completo (recovery) e Web Flasher (mais fácil).

## [3.3.6] — 2026-05-06

### Fixed
- **CI workflow `release.yml`**: corrigido step "Rename artifacts" que
  falhava com `mv: target 'rs50_thermal_v3.3.5.bin': No such file`.
  - Causa: `arduino-cli` salva binários em subpasta
    `./release/rs50_thermal/`, não direto em `./release/`.
  - Solução: usar `find ./release -name "*.bin"` para localizar.
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

## [3.3.5] — 2026-05-06

### Fixed
- **`config.h` faltando `PWM_CHANNEL_FAN`**: o `.ino` da v3.3.4 referenciava
  `PWM_CHANNEL_FAN` mas o header só definia `PWM_CHANNEL`. Build do CI
  falhava com `'PWM_CHANNEL_FAN' was not declared`. Renomeado o define
  para o nome usado no firmware.
- **Versão dessincronizada no `config.h`**:
  - Comentário do header dizia `Version: 3.3.0 (FINAL)` → atualizado para `3.3.5`.
  - `FW_VERSION` estava em `"3.3.2"` (defasado) → atualizado para `"3.3.5"`.
- **`README.md`**: seção "Estrutura do repositório" mencionava
  `docs/wiring-schematic-v3.3.1.svg` (deletado) e diretórios inexistentes
  (`firmware/`, `platformio.ini`). Atualizada para refletir a estrutura
  real do repo.

### Changed
- `.ino` header bumpado para `v3.3.5`.

## [3.3.4] — 2026-05-06

### Fixed
- **Patches do `.ino` aplicados de fato** (a v3.3.3 documentou mas não
  modificou o código). Confirmadas as 4 ocorrências com nova API:
  - `ledcSetup()` + `ledcAttachPin()` no setup
  - 3× `ledcWrite(PWM_CHANNEL_FAN, ...)` em vez de `PIN_PWM`
- **Comentários enganosos**: removidas referências a "Core 3.x" no
  header e na seção de ADC. Agora declaram corretamente Core 2.0.x.
- **`getStateColor()` e `getStateBrightness()`**: adicionado `default`
  em todos os switches (elimina warnings do compilador).
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
