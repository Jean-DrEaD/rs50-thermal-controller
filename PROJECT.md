# RS50 Thermal Controller — Documento Único de Contexto

> **Arquivo único de retomada de projeto.** Contém: identidade, hardware, pinout,
> firmware, CI/CD, lições aprendidas e changelog resumido. Atualize a cada release.
>
> **Última atualização:** 2026-05-14 · **Versão atual:** v3.3.13

---

## 0. 🚀 TL;DR pra retomar contexto

- **O que é:** controle térmico fail-safe pra volante RS50 (motor BLDC hoverboard 15Nm + MKS XDrive Mini com ODESC FFBeast).
- **MCU:** ESP32-S3-Zero (Waveshare, **sem PSRAM**).
- **Lógica:** NTC 100k no estator → ≥68°C corta +24V da MKS via relé NC.
- **Estado:** v3.3.13 publicada (fix CI: `UDP_HOST`/`UDP_PORT` faltando no `.example`). Bring-up de hardware em andamento.
- **Stack CI:** Arduino CLI + core `esp32:esp32@2.0.14` + FastLED 3.6.0 + WebSockets 2.4.1.
- **FQBN oficial:** `esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashMode=qio,FlashSize=4M,PartitionScheme=default,PSRAM=disabled`

---

## 1. 🎯 Objetivo e Filosofia

Controle térmico **fail-safe** — se algo der errado no controlador, o motor **continua funcionando**. O corte só acontece quando o sistema **conscientemente** decide cortar (NTC ≥ 68°C).

---

## 2. 🧩 Hardware (BOM fechada)

| Qtd | Componente | Notas |
|---|---|---|
| 1 | ESP32-S3-Zero (Waveshare) | MCU principal — **sem PSRAM** |
| 1 | Songle SLA-24VDC-SL-C | Relé NC SPDT 30A |
| 1 | IRLZ44N | MOSFET logic-level |
| 1 | 1N4007 | Diodo flyback (K=faixa no +24V) |
| 2 | HW-411 (LM2596) | 24→12V e 12→5V |
| 1 | NTC 100k B3950 | Encapsulado vidro, no estator |
| 1 | Fan 120mm 12V 4-pin PWM | 25kHz, PWM 3.3V direto |
| 1 | WS2812 strip (5 LEDs) | Status visual, 330Ω série |
| — | Resistores | 220Ω, 10k, 100k, 330Ω |
| — | Capacitor | 100nF (filtro NTC) |
| 1 | XT60 | Entrada 24V |

### Topologia fail-safe
- +24V → COM do relé → NC (fechado em repouso) → MKS VIN+
- GND vai **direto** da fonte pra MKS (não passa pelo relé)
- ESP travar / faltar 5V → relé volta a NC fechado → motor continua OK
- Shutdown: GPIO4=HIGH → MOSFET ON → bobina energiza → NC abre → motor para
- **OTA:** fan=0 e relay=LOW forçados (visual magenta no WS2812)

---

## 3. 🔌 Pinout ESP32-S3-Zero

| GPIO | Função | Macro | Notas |
|---|---|---|---|
| 1 | NTC ADC | `PIN_NTC` | Divisor 100k + cap 100nF |
| 4 | Gate MOSFET | `PIN_RELAY` | 220Ω + pull-down 10k, ativo HIGH |
| 5 | PWM Fan | `PIN_PWM` | 25kHz, 3.3V direto |
| 9 | WS2812 Data | `PIN_WS2812` | 330Ω série, `LED_COUNT=5` |
| 21 | ⚠️ NÃO USAR | — | LED RGB onboard |

> ⚠️ I2C **não usado** em v3.x (removido em v3.3.10).

---

## 4. 🌡 Thresholds Térmicos & FSM

| Faixa | Estado | Ação | Macro |
|---|---|---|---|
| < 40°C | IDLE | Fan OFF, LED azul | `TEMP_IDLE` |
| 40–60°C | WARMING | PWM proporcional, LED verde | `TEMP_WARMING` |
| 60–68°C | WARNING | PWM 100%, LED amarelo | `TEMP_WARNING` |
| ≥ 68°C | **CRITICAL** | Motor cortado, LED vermelho | `TEMP_CRITICAL` |
| < 63°C (pós-CRIT) | Religa | Histerese 5°C | `TEMP_RESTART` |

- `PWM_MIN = 0` (fan desliga em repouso, sem kick no cold boot)
- `THERMAL_HYSTERESIS = 2.0f` geral; 5°C grande só em CRITICAL→WARNING

---

## 5. ⚡ Flyback Diode (1N4007) — Regra de Ouro

> **Cátodo (faixa) sempre no lado +24V fixo, anodo no lado chaveado (Drain).**
> Mnemônico: **K = Katodo = faixa**.

- Operação normal: polarização reversa, **não conduz**.
- Desligamento: B2 sobe acima de B1+0,7V → diodo conduz → energia magnética dissipa em loop fechado em ~1ms.
- MOSFET nunca vê mais que ~24,7V no Drain.

---

## 6. 🌐 Rede e Telemetria

- **WiFi STA** — credenciais em `config.h` (template em `config.h.example`)
- **HTTP :80** — dashboard + `/status` (JSON com `fw`)
- **WS :81** — telemetria JSON em tempo real
- **UDP :33333** — CSV broadcast pra SimHub (`UDP_HOST=255.255.255.255`)
- **OTA** — `rs50-thermal.local:3232` (ArduinoOTA)
  - Upload: `tools/flash_ota.ps1` ou `tools/flash_ota.sh`
  - Visual: magenta=start, white=done, red=error

---

## 7. 🛠 Stack de Software

- **Arduino IDE 2.x** + `arduino-cli` no CI
- **ESP32 Arduino Core:** `2.0.14` (fixado — **nunca `latest`**)
- **FastLED:** 3.6.0 · **WebSockets:** 2.4.1 (em `ci/libraries.txt`)
- **PWM:** `ledcSetup` + `ledcAttachPin` (core 2.x) — ⚠️ NÃO usar `ledcAttach` (core 3.x)
- **OTA:** `ArduinoOTA` nativo
- **Dashboard:** HTML/CSS/JS em `dashboard.h` (raw literal C++)

---

## 8. 📂 Estrutura do Repositório

```
. ├── PROJECT.md ← este arquivo (fonte única de contexto) ├── README.md ← visão geral pública + diagramas Mermaid ├── CHANGELOG.md ← histórico Keep a Changelog ├── docs/ │ ├── banner.svg │ └── wiring-schematic.svg ← export Fritzing ├── src/rs50_thermal/ │ ├── rs50_thermal.ino ← firmware principal │ ├── config.h ← FW_VERSION + macros (NÃO COMMITAR) │ ├── config.h.example ← template público │ └── dashboard.h ← HTML/CSS/JS embutido ├── ci/ │ └── libraries.txt ← deps fixadas ├── tools/ │ ├── flash_ota.sh │ └── flash_ota.ps1 ├── hardware/ ← KiCad PCB (inativo no v3.x) └── .github/workflows/ ├── build.yml ← Arduino Build & Validate └── release.yml ← Release Build (tag v*..)
```
---

## 9. ⚙️ CI/CD

### `build.yml` — Build & Validate
- Disparo: push em `main`/`develop` (paths `src/**`, `ci/**`), PRs, manual.
- Instala core 2.0.14 + libs de `ci/libraries.txt`.
- Compila com FQBN oficial, warning se binário >1.25MB.
- Job `lint`: tabs, trailing whitespace, padrão `FW_VERSION 3.3.x`.

### `release.yml` — Release Build
- Disparo: push de tag `v*.*.*`.
- **Valida que a tag bate com `FW_VERSION`** (falha se divergir).
- Publica 3 binários: app (`*.bin`), `*_bootloader.bin`, `*_partitions.bin`.
- App grava em `0x10000`, **não** `0x0`.

### FQBN oficial
```
esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashMode=qio,FlashSize=4M,PartitionScheme=default,PSRAM=disabled
```

---
### Fluxo de release
1. Bump `FW_VERSION` em `config.h`
2. Atualizar header do `.ino` (linha 2 — pre-commit hook valida)
3. Atualizar `CHANGELOG.md` + este `PROJECT.md`
4. Branch `hotfix/X` ou `feat/X` (nunca direto na main)
5. PR → CI verde → squash merge
6. `git pull` → `git tag -a vX.Y.Z -F tag_msg.txt` → push tag
7. Release Build dispara automaticamente

---
## 10. 🎓 Lições Aprendidas (gotchas consolidados)

1. **Raw string delimiter ≤ 16 chars** (C++ standard). `RS50DASH338` ✅, `DASHBOARD_RS50_V338` ❌.
2. **HTML/JS em `.h` separado** — template literals `${}` confundem o parser do arduino-cli.
3. **Branch `main` protegida** — só via PR, "Require status checks" mantido.
4. **PowerShell + git commit:** usar `-F arquivo.txt` pra multi-linha.
5. **`git ls-remote` em tag anotada:** usar `refs/tags/vX.Y.Z^{}` pra resolver até o commit.
6. **Fixar versão do core ESP32 no CI** — nunca `latest`.
7. **Includes comentados ainda quebram CI** — arduino-cli resolve deps antes do pré-processador. Se não usa, **deleta**.
8. **Macros: single source of truth no `config.h`** — nunca redefinir no `.ino`.
9. **README pode prometer macros que o código não tem** — sempre grep depois de mexer.
10. **Pinout precisa bater com o MCU real** — S3-Zero ≠ ESP32 clássico.
11. **`FlashFreq` não é válido pro FQBN do esp32s3** — validar com `arduino-cli board details`.
12. **USB-CDC + auto-reset é frágil no S3-Zero** — ter fallback manual GPIO0+RESET. Preferir Tera Term/PuTTY.
13. **OTA precisa de fail-safe explícito** — forçar fan=0/relay=LOW em `onStart()`. Visual magenta/white/red ajuda em campo.
14. **CI não substitui macros no `config.h.example`** — é cópia literal. Todo símbolo usado no firmware **precisa** existir no `.example` com default público seguro. Validar com:
    ```powershell
    Select-String -Path src\rs50_thermal\*.ino -Pattern "^\s*[A-Z_]+\(?"
    ```

---

## 11. 📍 Status Atual e Próximos Passos

### Status
- **Software:** ✅ v3.3.13 (fix `UDP_HOST` no `.example`, CI verde)
- **Config:** ✅ pinout, thresholds e macros sincronizados
- **CI/Build:** ✅ core fixado 2.0.14, FQBN validado
- **Hardware:** 🔧 bring-up parcial (upload via GPIO0+RESET manual validado)

### Bring-up incremental (só avança se a anterior passar)
- [ ] 1. Reguladores isolados (12V e 5V sem carga)
- [ ] 2. + ESP32-S3 (boot, USB serial)
- [ ] 3. + NTC (ADC vs termômetro de referência)
- [ ] 4. + Fan PWM (off / ramp / max)
- [ ] 5. + WS2812 (cores por estado, LED_COUNT=5)
- [ ] 6. + Relé com carga fake (LED+resistor)
- [ ] 7. + MKS + motor real (fail-safe completo)
- [ ] 8. Calibração curva NTC (banho quente + padrão)
- [ ] 9. Validar histerese (>68°C → religa <63°C)
- [ ] 10. Dashboard web (Wi-Fi STA, real-time)
- [ ] 11. OTA em produção (`tools/flash_ota.ps1`)

### Fora de escopo (v3.x)
- Comunicação MKS XDrive Mini (**não tem UART** — só CAN/SPI). Planejado pós bring-up.
- Telemetria Wi-Fi/BLE avançada → v4.x
- PCB KiCad (existe em `hardware/` mas inativo)
- Carcaça/case

---

## 12. 📜 Changelog Resumido

> Detalhes completos em `CHANGELOG.md`. Aqui só as linhas mestras.

| Versão | Data | Resumo |
|---|---|---|
| **3.3.13** | 2026-05-14 | Fix CI: `UDP_HOST`/`UDP_PORT` adicionados ao `config.h.example`. Lição #14. |
| 3.3.12 | 2026-05-13 | OTA com fail-safe visual (magenta/white/red), boot banner, `fw` no payload, LED FSM real-time, PWM inicia em 0. |
| 3.3.11 | 2026-05-13 | `config.h.example` versionado, doc procedimento manual GPIO0+RESET, `flash.ps1`, FQBN consolidado (`PSRAM=disabled`), HTML em `web_ui.h`. |
| 3.3.10 | 2026-05-13 | Pinout migrado pro S3-Zero (1/4/5/9), NTC 10k→100k, thresholds alinhados, `LED_COUNT`/`PIN_WS2812`/`TEMP_RESTART` adicionados, I2C removido. |
| 3.3.9 | 2026-05-12 | Removido `#include <Adafruit_GFX.h>` órfão (mesmo comentado quebrava CI). Lição #7. |
| 3.3.8 | 2026-05-07 | `dashboard.h` separado, `ci/libraries.txt` criado, delimitador raw `RS50DASH338` (11 chars). |
| 3.3.7 | 2026-05-06 | Fix release publicando bootloader em vez do app. Agora 3 binários separados + gráfico Chart.js + pre-commit hook. |
| 3.3.6 | 2026-05-06 | Core ESP32 fixado em 2.0.14, FQBN `PSRAM=disabled`, actions atualizadas pra v2. |
| 3.3.5 | 2026-05-06 | `PWM_CHANNEL_FAN` no `config.h`, versões sincronizadas, README com estrutura real. |
| 3.3.4 | 2026-05-06 | Patches do `.ino` aplicados de fato (ledcSetup+ledcAttachPin), `getStateColor` com `default`, guards em `renderLEDs_Bar`, footer dashboard dinâmico, schematic Fritzing oficial. |
| 3.3.3 | 2026-05-06 | Migração API PWM core 3.x → 2.x (`ledcAttach` → `ledcSetup`+`ledcAttachPin`). |
| 3.3.2 | 2026-05-06 | Schematic Fritzing como fonte da verdade, `CONTEXT.md` consolidado, scripts locais removidos. |
| 3.3.1 | 2026-05-06 | Polaridade do flyback documentada (K=faixa no +24V), labels Anodo/Katodo, B1=VCC/B2=Drain. |
| 3.3.0 | 2026-05-05 | Topologia fail-safe oficial (Songle NC + IRLZ44N + 1N4007 + divisor NTC + WS2812). |
| 3.2.0 | 2026-05-03 | Cadeia HW-411 24→12→5V. DRV8871 substituído por relé (fail-safe). |
| 3.1.0 | 2026-05-01 | Primeiro protótipo ESP32-S3-Zero + NTC + cooler. |