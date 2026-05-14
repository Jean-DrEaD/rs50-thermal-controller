# RS50 Thermal Controller — Contexto de Sessão

> **Versão atual:** 3.3.13  
> **Última atualização:** 2026-05-14  
> Documento dinâmico — registra o **estado atual de trabalho**, decisões em aberto e próximos passos.  
> Para visão estável do projeto, ver [`PROJECT.md`](./PROJECT.md).

---

## 🎯 Foco da sessão atual

- Consolidar documentação (`README.md`, `PROJECT.md`, `CONTEXT.md`, `CHANGELOG.md`) para v3.3.13
- Validar fluxo de **bump automático do banner SVG** via workflow (`banner-bump.yml`)
- Garantir que o **OTA fail-safe** (fan=0 + relay=LOW + LED magenta) esteja estável
- Preparar terreno para **validação em hardware real** por etapas

---

## ✅ Checklist de validação por etapas

| Etapa | Item                                  | Status     | Notas                                  |
|-------|---------------------------------------|------------|----------------------------------------|
| 1     | Boot do ESP32-S3 + Serial             | ✅ OK      | 115200 baud, logs limpos               |
| 2     | Leitura NTC (ADC GPIO1)               | ✅ OK      | Divisor 100k + 100nF, curva B3950      |
| 3     | PWM Fan 25kHz (GPIO5)                 | ✅ OK      | `ledcSetup` + `ledcAttachPin`          |
| 4     | WS2812 5 LEDs (GPIO9)                 | ✅ OK      | FastLED 3.6.0, 330Ω série              |
| 5     | Relé via MOSFET (GPIO4)               | 🟡 Parcial | Bench OK, falta teste com motor real   |
| 6     | FSM Temperatura completa              | ✅ OK      | Histerese 5°C em CRITICAL        |
| 7     | WiFi STA + dashboard HTTP/WS          | ✅ OK      | porta 80/81                            |
| 8     | UDP broadcast SimHub (33333)          | ✅ OK      | JSON validado                          |
| 9     | OTA `rs50-thermal.local:3232`         | ✅ OK      | fail-safe ativo no `onStart()`         |
| 10    | Teste com motor 24V real              | ⏳ Pendente | Próxima etapa crítica                  |

---

## 📂 Arquivos em foco (linhas atuais)

| Arquivo                          | Estado    | Última alteração relevante               |
|----------------------------------|-----------|------------------------------------------|
| `src/rs50_thermal/rs50_thermal.ino` | ✅ Estável | OTA fail-safe + FSM completa             |
| `src/rs50_thermal/config.h`         | ✅ Estável | `FW_VERSION "3.3.13"`                    |
| `src/rs50_thermal/config.h.example` | ✅ Estável | Modelo sem credenciais                   |
| `src/rs50_thermal/dashboard.h`      | ✅ Estável | Raw string `R"RS50DASH338(...)"`         |
| `.github/workflows/build.yml`       | ✅ Estável | Core esp32 fixado em 2.0.14              |
| `.github/workflows/banner-bump.yml` | 🟡 Em ajuste | Validar query string `?v=X.Y.Z`        |
| `README.md`                          | ✅ Atualizado | Banner + diagramas mermaid             |
| `PROJECT.md`                         | ✅ Atualizado | Reescrito para v3.3.13                 |
| `CHANGELOG.md`                       | ✅ Atualizado | Entrada v3.3.13 consolidada            |

---

## 🧠 Decisões recentes

- **I2C removido permanentemente** desde v3.3.10 (`Wire.h`, `PIN_SDA`, `PIN_SCL` excluídos)
- **Core esp32 fixado em 2.0.14** — não migrar para 3.x sem refator de `ledc*` API
- **Delimitador raw string** limitado a 16 chars → padrão adotado: `RS50DASH<versão_curta>`
- **Branch `main` protegida** — só merge via PR com CI verde
- **Banner SVG** versionado via query string para invalidar cache do GitHub

---

## ⚠️ Pontos de atenção

- `arduino-cli` no CI **não pode** usar `core esp32:esp32@latest` — sempre `@2.0.14`
- HTML/JS grandes devem ficar em arquivo `.h` separado, **nunca inline no `.ino`**
- Ao criar nova tag, lembrar de rodar `git ls-remote` com `refs/tags/vX.Y.Z^{}` para resolver commit real
- Durante OTA, **forçar** `fan=0` e `relay=LOW` no `onStart()` — falha aqui = motor sem corte

---

## 🚧 Tarefas em aberto
- [ ] Testar `banner-bump.yml` em tag de teste (`v3.3.13-rc1`)
- [ ] Validar relé + motor real (24V) em bancada
- [ ] Documentar curva NTC B3950 com tabela de calibração em `docs/`
- [ ] Criar script `tools/ota_flash.sh` parametrizado por versão
- [ ] Avaliar inclusão de watchdog hardware (TPL5010) em revisão futura

---

## 🔁 Comandos úteis da sessão

```bash
# Build local
arduino-cli compile --fqbn esp32:esp32:esp32s3 src/rs50_thermal

# Upload via OTA
python tools/espota.py -i rs50-thermal.local -p 3232 -f build/rs50_thermal.ino.bin

# Resolver commit de uma tag anotada
git ls-remote origin "refs/tags/v3.3.13^{}"

# Criar tag e push
git tag -a v3.3.13 -m "Release v3.3.13"
git push origin v3.3.13

# Forçar refresh do banner (cache GitHub)
# Atualizar README.md: ![banner](docs/banner.svg?v=3.3.13)
