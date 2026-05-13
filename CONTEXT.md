# RS50 Thermal Controller — Context

> Documento curto para retomar o projeto rapidamente.
> Última atualização: 2026-05-13 (v3.3.12).

## 🎯 Objetivo

Controle térmico **fail-safe** para volante RS50 com motor BLDC de
hoverboard (15Nm) + MKS XDrive Mini (firmware ODESC FFBeast). Corta o
+24V da MKS quando NTC ≥ 68°C.

## 📍 Status atual

- **Software**: ✅ v3.3.12 publicada — OTA + FSM LEDs + boot banner
- **Config**: ✅ pinout, thresholds e macros sincronizados com hardware real
- **CI/Build**: ✅ core ESP32 fixado em `2.0.14`, FQBN validado com `PSRAM=disabled`
- **Hardware**: 🔧 bring-up parcial — upload via procedimento manual GPIO0+RESET validado
- **Próximo passo**: validar boot limpo + serial CDC → seguir checklist de bring-up

## 🧩 Hardware fechado

- **MCU**: ESP32-S3-Zero (Waveshare)
- **Sensor**: NTC 100k B3950 (vidro) no estator
- **Atuador**: Relé Songle SLA-24VDC-SL-C (NC, SPDT 30A)
- **Driver**: IRLZ44N + 220Ω no gate + pull-down 10k + flyback 1N4007
- **Reguladores**: 2× HW-411 (LM2596): 24→12V e 12→5V
- **Cooler**: Fan 120mm 12V 4-pin PWM nativo (PWM 3.3V direto do GPIO)
- **Status**: WS2812 (5 LEDs) com 330Ω série

## 🔌 Pinout ESP32-S3-Zero

| GPIO | Função      | Macro         | Notas                              |
|------|-------------|---------------|------------------------------------|
| 1    | NTC ADC     | `PIN_NTC`     | divisor 100k + cap 100nF           |
| 4    | Gate MOSFET | `PIN_RELAY`   | 220Ω + pull-down 10k, ativo HIGH   |
| 5    | PWM Fan     | `PIN_PWM`     | 25kHz, 3.3V direto                 |
| 9    | WS2812 Data | `PIN_WS2812`  | 330Ω série, `LED_COUNT=5`          |
| 21   | ⚠️ NÃO USAR | —             | LED RGB onboard                    |

> ⚠️ I2C **não é usado** em v3.x. `Wire.h`, `PIN_SDA`, `PIN_SCL` foram
> removidos em v3.3.10.

## 🛡 Topologia fail-safe

- +24V → COM do relé → NC (fechado em repouso) → MKS VIN+
- GND vai **direto** da fonte pra MKS (não passa pelo relé)
- ESP travar / faltar 5V → relé volta a NC fechado → motor continua OK
- Shutdown: GPIO4=HIGH → MOSFET ON → bobina energiza → NC abre → motor para
- **Durante OTA**: fan=0 e relay=LOW forçados (fail-safe visual magenta)

## 🌡 Thresholds de temperatura

| Faixa             | Estado       | Ação                          | Macro            |
|-------------------|--------------|-------------------------------|------------------|
| < 40°C            | IDLE         | Fan OFF, LED azul             | `TEMP_IDLE`      |
| 40–60°C           | WARMING      | PWM proporcional, LED verde   | `TEMP_WARMING`   |
| 60–68°C           | WARNING      | PWM 100%, LED amarelo         | `TEMP_WARNING`   |
| ≥ 68°C            | **CRITICAL** | Motor cortado, LED vermelho   | `TEMP_CRITICAL`  |
| < 63°C (pós-CRIT) | Religa       | Histerese de 5°C              | `TEMP_RESTART`   |

- `PWM_MIN = 0` (fan desliga em repouso)
- `THERMAL_HYSTERESIS = 2.0f` (geral); 5°C grande só em CRITICAL→WARNING

## 🌐 Rede

- **WiFi STA**  — credenciais em `config.h` (use `config.h.example`)
- **HTTP :80**  — dashboard + `/status` (JSON com `fw`)
- **WS :81**   — telemetria JSON em tempo real
- **UDP 33333** — CSV broadcast para SimHub
- **OTA**       — `rs50-thermal.local:3232` (ArduinoOTA)
  - Upload: `tools/flash_ota.ps1` ou `tools/flash_ota.sh`
  - Visual: magenta=start, white=done, red=error

## 🛠 Stack de software

- **Arduino IDE 2.x** + `arduino-cli` no CI (GitHub Actions)
- **ESP32 Arduino Core**: `2.0.14` (fixado nos workflows)
- **FastLED**: 3.6.x (WS2812) — versão fixada inline nos workflows
- **PWM**: API `ledcSetup` + `ledcAttachPin` (core 2.x)
  ⚠️ NÃO usar `ledcAttach` (essa é da core 3.x)
- **OTA**: `ArduinoOTA` (nativo do core esp32)
- **Dashboard Web**: HTML/CSS/JS embutido em `dashboard.h` (raw literal C++)
- **Telemetria**: WebSocket (porta 81) + UDP broadcast (porta 33333 para SimHub)


## 📂 Estrutura

```
.
├── README.md ← visão geral + diagramas Mermaid + banner 
├── CHANGELOG.md ← histórico (Keep a Changelog) 
├── CONTEXT.md ← este arquivo 
├── docs/ 
│    ├── banner.svg ← banner do projeto (usado no README) 
│    └── wiring-schematic.svg ← export Fritzing breadboard 
├── src/ 
│    └── rs50_thermal/ 
│        ├── rs50_thermal.ino ← firmware principal 
│        ├── config.h ← FW_VERSION + macros 
│        └── dashboard.h ← HTML/CSS/JS do dashboard web 
├── hardware/ ← KiCad PCB (sem planos ativos no v3.x) 
└── .github/ 
     └── workflows/ ← CI (Arduino Build & Validate + Release Build)

```


## 🎓 Lições aprendidas (gotchas do build)

### 1. Raw string delimiter tem limite de 16 caracteres
C++ standard: o delimitador entre `R"` e `(` aceita **no máximo 16 chars**.
- ✅ `R"RS50DASH338(...)RS50DASH338"` (11 chars)
- ❌ `R"DASHBOARD_RS50_V338(...)DASHBOARD_RS50_V338"` (19 chars)

### 2. HTML/JS embutido: separar em `.h` próprio
Manter HTML grande dentro do `.ino` confunde o parser do `arduino-cli` quando há
template literals JS (`${...}`).
- Separar HTML em `dashboard.h`
- Evitar template literals — usar concatenação `+`
- Evitar arrow functions complexas — preferir `function() {...}`
- Delimitador raw literal único e curto (≤16 chars)

### 3. Branch `main` é protegida — só via PR
- "Require approvals" desabilitado (single-maintainer repo)
- "Require status checks" mantido (CI tem que passar)
- Fluxo: `git checkout -b hotfix/X` → push → PR → CI verde → merge

### 4. PowerShell + git commit: usar `-F arquivo.txt`
Mensagens multi-linha com `git commit -m` sofrem com escape de aspas e
caracteres especiais no PowerShell. Sempre escrever em arquivo temporário.

### 5. `git ls-remote` em tags anotadas retorna o objeto da tag
Pra resolver até o commit: `git ls-remote origin "refs/tags/v3.3.8^{}"`

### 6. Fixar versão do core ESP32 no CI
`arduino-cli core install esp32:esp32@2.0.14` — nunca usar `latest`.
Mesma regra vale pras libs.

### 7. Includes comentados ainda quebram o CI
O arduino-cli resolve dependências **antes** do pré-processador. Um
`// #include <Adafruit_GFX.h>` comentado pode causar falha de build.
**Regra**: se não usa, deleta a linha.

### 8. Macros redefinidas em `.ino` × `config.h`
Single source of truth: **sempre no `config.h`**. Nunca redefinir no `.ino`.

### 9. README pode prometer macros que o código não tem
Sempre que mexer no README mencionando configuração, fazer grep no `config.h`.

### 10. Pinout do `config.h` precisa bater com o MCU real
Pinos diferem entre famílias ESP32 — sempre validar contra datasheet.

### 11. `FlashFreq` não é parâmetro válido do FQBN para `esp32s3`
Validar opções com `arduino-cli board details -b esp32:esp32:esp32s3`.

### 12. USB-CDC nativo + auto-reset é frágil no S3-Zero
Com `USBMode=hwcdc` + `CDCOnBoot=cdc`, o handshake DTR/RTS falha em ~30%
das tentativas. Sempre ter fallback manual (GPIO0 + RESET) documentado.
Preferir Tera Term/PuTTY ao `arduino-cli monitor` pra debug serial pós-flash.

### 13. OTA precisa de fail-safe explícito (v3.3.12)
Durante `ArduinoOTA.onStart()`, **forçar** fan=0 e relay=LOW antes do flash.
Sem isso, o motor pode ficar acionado se o ESP travar no meio do update.
Feedback visual no WS2812 (magenta/white/red) ajuda no debug em campo.

## 🔄 Fluxo de release validado

1. Bump `FW_VERSION` em `src/rs50_thermal/config.h`
2. Atualizar header do `.ino` (linha 2 — validado pelo pre-commit hook)
3. Atualizar `CHANGELOG.md`
4. Commit em branch `hotfix/X` ou `feat/X` (nunca direto na main)
5. Push da branch → abrir PR (`gh pr create --fill --base main`)
6. Aguardar CI verde (Arduino Build & Validate)
7. Merge na main via squash (`gh pr merge N --squash --delete-branch`)
8. `git pull` + criar `tag_msg.txt` + `git tag -a vX.Y.Z -F tag_msg.txt`
9. `git push origin vX.Y.Z`
10. Workflow Release Build dispara automaticamente

> 💡 Recriar tag após hotfix: `git tag -d vX.Y.Z` + `git push origin :refs/tags/vX.Y.Z`,
> corrigir, merge, então `git tag -a vX.Y.Z -F tag_msg.txt && git push origin vX.Y.Z`.

## 📡 Configuração de build validada

```powershell
$fqbn = "esp32:esp32:esp32s3:" + `
        "USBMode=hwcdc," + `
        "CDCOnBoot=cdc," + `
        "FlashMode=qio," + `
        "FlashSize=4M," + `
        "PartitionScheme=default," + `
        "PSRAM=disabled"
```

## ❌ Fora de escopo (v3.x)

- Comunicação com a MKS XDrive Mini
  * ⚠️ A placa **não suporta UART** — apenas **CAN** ou **SPI**
  * Telemetria bidirecional planejada apenas após bring-up completo
- Telemetria via Wi-Fi/BLE (planejado v4.x — após hardware testado)
- Migração para PCB (KiCad em `hardware/` existe mas **sem planos ativos**)
- Carcaça/case


## 🚧 Próximas etapas (hardware bring-up)

Ordem sugerida pra debug incremental — só avança se a anterior passar:

- [ ] **1. Reguladores isolados**: mede 12V e 5V com multímetro (sem carga)
- [ ] **2. + ESP32-S3**: confirma boot, USB serial responde
- [ ] **3. + NTC**: valida ADC contra termômetro de referência
- [ ] **4. + Fan PWM**: testa 3 faixas (off / ramp / max)
- [ ] **5. + WS2812**: confirma cores em cada estado (`LED_COUNT=5`)
- [ ] **6. + Relé com carga fake** (LED+resistor): valida shutdown
- [ ] **7. + MKS + motor real**: teste fail-safe completo
- [ ] **8. Calibração da curva NTC**: banho quente + termômetro padrão
- [ ] **9. Validar histerese**: aquece >68°C, espera religar em <63°C
- [ ] **10. Dashboard web**: Wi-Fi STA mode, leitura em tempo real
- [ ] **11. OTA em produção**: validar tools/flash_ota.ps1 com firmware real

## 🐛 O que foi corrigido no v3.3.12

**1. OTA com fail-safe visual** — fan/relay zerados durante flash, feedback RGB.
**2. Boot banner** — versão, build date e Chip ID no boot serial.
**3. Firmware no payload** — telemetria WS e /status agora carregam a versão.
**4. LED strip mirrors FSM** — cores em tempo real (azul/verde/amarelo/vermelho).
**5. PWM inicia em 0** — evita "kick" do fan no cold boot.


### 📡 Telemetria Completa (apenas após bring-up completo)

Implementar **somente depois** que tudo acima estiver validado em bancada:

- Decidir entre CAN (preferido — protocolo nativo da MKS) ou SPI
- Provavelmente CAN: ESP32-S3 tem TWAI nativo, MCP2551 como transceiver
- Ler temperatura/erro/status do ODESC FFBeast
- Expor tudo no dashboard web e/ou enviar via Wi-Fi/BLE
