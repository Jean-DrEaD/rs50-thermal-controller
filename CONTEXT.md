# RS50 Thermal Controller — Context

> Documento curto para retomar o projeto rapidamente.
> Última atualização: 2026-05-12 (v3.3.8 + CI refactor).

## 🎯 Objetivo

Controle térmico **fail-safe** para volante RS50 com motor BLDC de
hoverboard (15Nm) + MKS XDrive Mini (firmware ODESC FFBeast). Corta o
+24V da MKS quando NTC ≥ 68°C.

## 📍 Status atual

- **Software**: ✅ v3.3.8 publicada, CI verde, release oficial no GitHub
- **CI/Build**: ✅ core ESP32 fixado em `2.0.14`, libs centralizadas em `ci/libraries.txt`
- **Hardware**: 🔧 aguardando montagem física e bring-up
- **Próximo passo**: montar protótipo na bancada → validar fail-safe → debug

## 🧩 Hardware fechado

- **MCU**: ESP32-S3-Zero (Waveshare)
- **Sensor**: NTC 100k B3950 (vidro) no estator
- **Atuador**: Relé Songle SLA-24VDC-SL-C (NC, SPDT 30A)
- **Driver**: IRLZ44N + 220Ω no gate + pull-down 10k + flyback 1N4007
- **Reguladores**: 2× HW-411 (LM2596): 24→12V e 12→5V
- **Cooler**: Fan 120mm 4-pin PWM nativo (3.3V direto do GPIO)
- **Status**: WS2812 (1–5 LEDs) com 330Ω série

## 🔌 Pinout ESP32-S3-Zero

| GPIO | Função       | Notas                          |
|------|--------------|--------------------------------|
| 1    | NTC ADC      | divisor 100k + cap 100nF       |
| 4    | Gate MOSFET  | 220Ω + pull-down 10k           |
| 5    | PWM Fan      | 25kHz, 3.3V direto             |
| 9    | WS2812 Data  | 330Ω série                     |
| 21   | ⚠️ NÃO USAR  | LED RGB onboard                |

## 🛡 Topologia fail-safe

- +24V → COM do relé → NC (fechado em repouso) → MKS VIN+
- GND vai **direto** da fonte pra MKS (não passa pelo relé)
- ESP travar / faltar 5V → relé volta a NC fechado → motor continua OK
- Shutdown: GPIO4=HIGH → MOSFET ON → bobina energiza → NC abre → motor para

## 🌡 Thresholds de temperatura

| Faixa                  | Estado     | Ação                          |
|------------------------|------------|-------------------------------|
| < 40°C                 | Repouso    | Fan OFF, LED verde            |
| 40–60°C                | Fan Ramp   | PWM proporcional, LED azul    |
| 60–68°C                | Fan Max    | PWM 100%, LED amarelo         |
| ≥ 68°C                 | **Shutdown** | Motor cortado, LED vermelho |
| < 63°C (pós-shutdown)  | Religa     | Histerese de 5°C              |

## 🛠 Stack de software

- **Arduino IDE 2.x** + `arduino-cli` no CI (GitHub Actions)
- **ESP32 Arduino Core**: `2.0.14` (fixado — ver `ci/libraries.txt` e `build.yml`)
- **FastLED**: 3.6.x (WS2812) — versão fixada em `ci/libraries.txt`
- **PWM**: API `ledcSetup` + `ledcAttachPin` (core 2.x)
  ⚠️ NÃO usar `ledcAttach` (essa é da core 3.x)
- **Dashboard Web**: HTML/CSS/JS embutido em `dashboard.h` (raw literal C++)

## 📂 Estrutura

```text
.
├── README.md                       ← visão geral + diagramas Mermaid + banner
├── CHANGELOG.md                    ← histórico (Keep a Changelog)
├── CONTEXT.md                      ← este arquivo
├── ci/
│   └── libraries.txt               ← libs + versões fixadas usadas no CI
├── docs/
│   ├── banner.svg                  ← banner do projeto (usado no README)
│   └── wiring-schematic.svg        ← export Fritzing breadboard
├── src/
│   └── rs50_thermal/
│       ├── rs50_thermal.ino        ← firmware principal
│       ├── config.h                ← FW_VERSION + macros
│       └── dashboard.h             ← HTML/CSS/JS do dashboard web
├── hardware/                       ← KiCad PCB (sem planos ativos no v3.x)
└── .github/
    └── workflows/                  ← CI (Arduino Build & Validate + Release Build)
```

## 🎓 Lições aprendidas (gotchas do build)

Dor de cabeça pra publicar v3.3.7/v3.3.8 deixou estes aprendizados:

### 1. Raw string delimiter tem limite de 16 caracteres

C++ standard: o delimitador entre `R"` e `(` aceita **no máximo 16 chars**.

- ✅ `R"RS50DASH338(...)RS50DASH338"` (11 chars)
- ❌ `R"DASHBOARD_RS50_V338(...)DASHBOARD_RS50_V338"` (19 chars)
- Erro: `error: raw string delimiter longer than 16 characters`

### 2. HTML/JS embutido: separar em `.h` próprio

Manter HTML grande dentro do `.ino` confunde o parser do `arduino-cli`
quando há template literals JS (`${...}`). Sintomas estranhos como
`error: 'function' does not name a type` aparecem em linhas que **não
têm nada a ver** com o problema real.

**Regras pro JS embutido:**
- Separar HTML em `dashboard.h` próprio
- Evitar template literals (backticks + `${}`) — usar concatenação `+`
- Evitar arrow functions complexas — preferir `function() {...}`
- Delimitador raw literal único e curto (≤16 chars)

### 3. Branch `main` é protegida — só via PR

- "Require approvals" **desabilitado** (single-maintainer repo)
- "Require status checks" **mantido** (CI tem que passar)
- Fluxo: `git checkout -b hotfix/X` → push → PR → CI verde → merge

### 4. PowerShell + git commit: usar `-F arquivo.txt`

Mensagens multi-linha com `git commit -m` sofrem com escape de aspas e
caracteres especiais no PowerShell. Sempre escrever em arquivo temporário
e usar `git commit -F commit_msg.txt`.

### 5. `git ls-remote` em tags anotadas retorna o objeto da tag

Não o commit que ela aponta. Pra resolver até o commit:
`git ls-remote origin "refs/tags/v3.3.8^{}"`

### 6. Fixar versão do core ESP32 no CI

Builds "verdes" hoje podem quebrar amanhã se a Espressif lançar core 3.x e o
CI pegar a `latest`. **Sempre fixar** a versão exata:

```yaml
- run: arduino-cli core install esp32:esp32@2.0.14
```

Mesma regra vale pras libs — centralizadas em `ci/libraries.txt` (uma por
linha no formato `Nome@versão`) e instaladas em loop pelo workflow. Isso
garante reprodutibilidade total entre máquina local e CI.

## 🔄 Fluxo de release validado (v3.3.8)

1. Bump `FW_VERSION` em `src/rs50_thermal/config.h`
2. Commit em branch `hotfix/X` ou `feat/X` (nunca direto na main)
3. Push da branch → abrir PR
4. Aguardar CI verde (Arduino Build & Validate)
5. Merge na main (Squash recomendado)
6. `git pull` + `git tag -a vX.Y.Z -F tag_msg.txt`
7. `git push origin vX.Y.Z`
8. Workflow Release Build dispara automaticamente
9. Release publicada com `.bin` anexado

## ❌ Fora de escopo (v3.x)

- Comunicação com a MKS XDrive Mini
  - ⚠️ A placa **não suporta UART** — apenas **CAN** ou **SPI**
  - Telemetria bidirecional planejada apenas após bring-up completo
- Telemetria via Wi-Fi/BLE (planejado v4.x — após hardware testado)
- Migração para PCB (KiCad em `hardware/` existe mas **sem planos ativos**)
- Carcaça/case

## 🚧 Próximas etapas (hardware bring-up)

Ordem sugerida pra debug incremental — só avança se a anterior passar:

- [ ] **1. Reguladores isolados**: mede 12V e 5V com multímetro (sem carga)
- [ ] **2. + ESP32-S3**: confirma boot, USB serial responde
- [ ] **3. + NTC**: valida ADC contra termômetro de referência
- [ ] **4. + Fan PWM**: testa 3 faixas (off / ramp / max)
- [ ] **5. + WS2812**: confirma cores em cada estado
- [ ] **6. + Relé com carga fake** (LED+resistor): valida shutdown
- [ ] **7. + MKS + motor real**: teste fail-safe completo
- [ ] **8. Calibração da curva NTC**: banho quente + termômetro padrão
- [ ] **9. Validar histerese**: aquece >68°C, espera religar em <63°C
- [ ] **10. Dashboard web**: Wi-Fi STA mode, leitura em tempo real

### 📡 Telemetria (apenas após bring-up completo)

Implementar **somente depois** que tudo acima estiver validado em bancada:

- Decidir entre CAN (preferido — protocolo nativo da MKS) ou SPI
- Provavelmente CAN: ESP32-S3 tem TWAI nativo, MCP2551 como transceiver
- Ler temperatura/erro/status do ODESC FFBeast
- Expor no dashboard web e/ou enviar via Wi-Fi/BLE




# Contexto — Hotfix v3.3.9

## Resumo
Correção emergencial de falha de build no CI da release **v3.3.9** do projeto
**rs50_thermal**, causada por um include órfão da biblioteca `Adafruit_GFX.h`.

## Linha do tempo
1. Release `v3.3.9` foi tagueada com um commit que continha a linha
   `//#include <Adafruit_GFX.h>` em `src/rs50_thermal/rs50_thermal.ino`.
2. O CI falhou ao tentar resolver a dependência inexistente no projeto.
3. A tag `v3.3.9` foi apagada (local e remota) para permitir correção limpa.
4. Branch `hotfix/v3.3.9-missing-include` criada a partir da `main`.
5. Linha removida, commit assinado e PR aberto.
6. Após CI verde, merge na `main` e recriação da tag `v3.3.9`.

## Arquivo afetado
- `src/rs50_thermal/rs50_thermal.ino` (linha 12 removida)

## Decisão técnica
O projeto **não utiliza display gráfico** e não há roadmap previsto para
integração com `Adafruit_GFX`. Manter o include — mesmo comentado — é
desnecessário e fonte de ruído no pipeline. Decisão: **remover por completo**.

## Comandos relevantes

### Apagar tag incorreta
\`\`\`bash
git tag -d v3.3.9
git push origin :refs/tags/v3.3.9
\`\`\`

### Criar branch de hotfix
\`\`\`bash
git checkout main
git pull
git checkout -b hotfix/v3.3.9-missing-include
\`\`\`

### Commit do fix
\`\`\`bash
git commit -m "fix(build): remove orphan Adafruit_GFX include breaking CI"
\`\`\`

### Recriar a tag após merge
\`\`\`bash
git checkout main
git pull
git tag -a v3.3.9 -m "Release v3.3.9 — hotfix orphan include"
git push origin v3.3.9
\`\`\`

### Verificação final
\`\`\`powershell
gci src -Recurse -Include *.ino,*.h,*.cpp | sls "Adafruit_GFX"
# Saída esperada: vazia
\`\`\`

## Impacto
- ✅ Build do CI restaurado
- ✅ Release `v3.3.9` consistente
- ✅ Codebase mais limpo, sem dependências fantasmas

## Lições aprendidas
- Comentar includes não os torna inertes em toda toolchain — **remover é mais seguro**.
- Antes de tagar uma release, rodar o CI localmente em modo "release" reduz retrabalho.
- Sempre validar tags com `git show <tag>` antes do push final.
