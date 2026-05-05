<div align="center">

# 🌡️ RS50 Thermal Controller

### Sistema inteligente de protecao termica para volantes Direct Drive

[![Build Status](https://github.com/Jean-DrEaD/rs50-thermal-controller/actions/workflows/arduino-build.yml/badge.svg)](https://github.com/Jean-DrEaD/rs50-thermal-controller/actions/workflows/arduino-build.yml)
[![Release](https://img.shields.io/github/v/release/Jean-DrEaD/rs50-thermal-controller?color=brightgreen&label=release)](https://github.com/Jean-DrEaD/rs50-thermal-controller/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Made with Arduino](https://img.shields.io/badge/Made%20with-Arduino-00979D?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Sim Racing](https://img.shields.io/badge/community-Sim%20Racing-red)](https://www.reddit.com/r/simracing/)

**Protecao termica fail-safe • Dashboard WiFi em tempo real • Estimativa adaptativa do estator**

[📥 Download](#-download--instalacao) •
[🔧 Hardware](#-hardware) •
[📊 Dashboard](#-dashboard-web) •
[🌡️ Calibracao](docs/calibration-guide.md) •
[🤝 Contribuir](#-contribuindo)

---

<img src="docs/banner.svg" alt="RS50 Thermal Controller" width="100%"/>

</div>

## 🎯 O Problema

Volantes **Direct Drive** com motores hoverboard 15Nm geram **calor extremo** durante sessoes longas de drift, oval ou track. As bobinas do estator podem atingir **80°C+**, causando:

- 🔥 **Degradacao do verniz** das bobinas (perda de isolacao)
- 🧲 **Desmagnetizacao** dos imas N35 (perda permanente de torque)
- ⚡ **Saturacao magnetica** (sensacao de "amolecimento" do FFB)
- 💥 **Falha catastrofica** em casos extremos

> **Solucoes comerciais custam R$ 800+ e sao caixas pretas.**
> Este projeto resolve por **menos de R$ 150** com hardware off-the-shelf e firmware open-source.

---

## ✨ Features

<table>
<tr>
<td width="50%">

### 🛡️ Protecao
- ✅ **Rele fail-safe** (NC) — se o ESP travar, motor continua ligado
- ✅ **Estimativa adaptativa** do estator (com `THERMAL_OFFSET` calibravel)
- ✅ **Lead time** antecipa rampas termicas (`dT/dt`)
- ✅ **3 niveis de protecao**: Warning -> Critical -> Shutdown
- ✅ **Latch de seguranca** (60s) apos desligamento

</td>
<td width="50%">

### 📊 Monitoramento
- ✅ **Dashboard WiFi** com WebSocket em tempo real
- ✅ **LEDs WS2812 configuraveis** (1, 3, 5+ LEDs)
- ✅ **Contador de horas** persistente em NVS
- ✅ **Pico de temperatura** da sessao
- ✅ **Deteccao FAULT** com grace period adaptativo

</td>
</tr>
<tr>
<td width="50%">

### 🎛️ Controle
- ✅ **PWM 25 kHz** (acima do audivel) para fan 4-pin Intel
- ✅ **Histerese** evita oscilacao on/off
- ✅ **Filtro EMA** reduz ruido do ADC
- ✅ **3 modos de LED**: Single / Bar / Thermometer

</td>
<td width="50%">

### 🚀 DevOps
- ✅ **GitHub Actions** com build automatico
- ✅ **Releases** com firmware `.bin` pronto
- ✅ **Templates** de Issue e PR
- ✅ **Semantic versioning** + Changelog
- ✅ **Documentacao completa** em PT-BR

</td>
</tr>
</table>

---

## 🏗️ Arquitetura

```
+-----------------------------------------------------------------+
|                    Fonte 24V (XDrive Mini)                      |
+--------------------------+--------------------------------------+
                           | XT60 Macho (chicote da fonte)
                           v
                      [XT60 Femea]  (PCB-mount)
                           |
              +------------+------------+
              | 24V                     | 24V (paralelo)
              v                         v
        +-----------+              +-----------+
        | HW-411 #1 |              |   ODESC   |
        | LM2596    | 12V          | FFBeast   |
        | 24V->12V  |--+---> Fan   |           |
        +-----+-----+  |   120mm   +-----+-----+
              |        |   PWM           | AC out
              | 12V    |                 v
              v        |          +-----------+
        +-----------+  |          |  Motor    |
        | HW-411 #2 |  |          | Hoverbrd  |
        | LM2596    |  |          |  15Nm     |
        | 12V->5V   |  |          +-----+-----+
        +-----+-----+  |                |
              | 5V     | 12V            | (chaveado pelo rele)
              v        v                |
        +-----------+ +---------+       |
        | ESP32-S3  | |  Songle |<------+
        |   Zero    | | Rele NC |
        |           | | 24VDC   |
        |  GPIO1 <--+-|---------|------ NTC 100K (eixo motor)
        |  GPIO5  ----|----------------> Fan PWM (azul, 25kHz)
        |  GPIO4  ----|----------------> MOSFET IRLZ44N (gate)
        |  GPIO21 ----|----------------> WS2812B (DIN)
        +-----------+ +---------+
```

📐 **[Diagrama detalhado em SVG](docs/wiring-schematic.svg)** • **[ASCII text](docs/wiring-ascii.txt)** • **[Fonte Fritzing](hardware/rs50-thermal.fzz)**

---

## 🔧 Hardware

### 📦 BOM (Bill of Materials)

| Qtd | Componente | Especificacao | Preco aprox. |
|:---:|---|---|---:|
| 1 | **MCU** | Waveshare ESP32-S3-Zero | R$ 35 |
| 1 | **Sensor** | NTC 100K B3950 (vidro) | R$ 5 |
| 1 | **Rele** | Songle SLA-24VDC-SL-C (SPDT, 30A) | R$ 18 |
| 2 | **Buck LM2596** | Modulo HW-411 (24V->12V e 12V->5V) | R$ 16 |
| 1 | **MOSFET driver** | IRLZ44N + R 220R + R 10K (pulldown) | R$ 4 |
| 1 | **Fan** | 12V 120mm PWM 4-pin (Intel-style) | R$ 25 |
| 1 | **Conector entrada** | XT60 Femea PCB-mount | R$ 6 |
| 1-5 | **LEDs** | WS2812B onboard + opcional strip | R$ 2-10 |
| 1 | **Resistor** | 100K 1% (divisor NTC) | R$ 0.50 |
| 1 | **Capacitor** | 100nF ceramico (filtro ADC) | R$ 0.50 |
| 1 | **Diodo flyback** | 1N4007 (protecao do rele) | R$ 0.50 |
| - | **Conectores** | JST-XH, terminais ring | R$ 10 |
| - | **Caixa** | Impressa 3D ou PVC | R$ 10 |
| | | **Total estimado** | **~R$ 145** |

📋 **[BOM completa em CSV](hardware/bom.csv)**

> 💡 **Por que XT60?** Conector padrao da industria de drones/RC, suporta ate 60A continuo, com pinos protegidos contra curto.
> 💡 **Por que 2x HW-411?** Cascata 24V->12V->5V tem melhor regulacao termica que conversao direta 24V->5V.

### 🎯 Compatibilidade

| Componente | Testado | Compativel |
|---|:---:|:---:|
| **RS50 Clone DD wheel** (XDrive Mini) | ✅ | ✅ |
| **Hoverboard motor 15Nm** | ✅ | ✅ |
| **ODESC FFBeast** | ✅ | ✅ |
| **ODrive v3.6** | ❌ | ⚠️ Provavelmente OK |
| **MaxxMotor (similar)** | ❌ | ⚠️ Adaptar conexoes |
| **Outros DDs com fonte 24V** | ❌ | ⚠️ Adaptar Buck |

### 🔌 Pinout ESP32-S3-Zero

| GPIO | Funcao | Direcao | Nota |
|:---:|---|:---:|---|
| **GPIO1** | NTC (ADC1_CH0) | Analog IN | Divisor com 100K + filtro 100nF |
| **GPIO4** | Gate MOSFET (rele) | Digital OUT | Via R 220R + pulldown 10K |
| **GPIO5** | Fan PWM (fio azul) | Digital OUT | 25 kHz, 8 bits, min 40% duty |
| **GPIO21** | LED WS2812 (DIN) | Digital OUT | Onboard + strip externa opcional |

> ⚠️ **Cabo verde do fan (TACH) NAO e utilizado** — cortar e isolar com termo-retratil.

---

## 📥 Download & Instalacao

### 🚀 Opcao 1 — Firmware Pre-compilado (mais facil)

1. Baixe a ultima `.bin` da [pagina de releases](https://github.com/Jean-DrEaD/rs50-thermal-controller/releases/latest)
2. Use o **[ESP Web Tools](https://espressif.github.io/esptool-js/)** no navegador (Chrome/Edge):
   - Conecte o ESP32-S3-Zero via USB-C
   - Clique em "Connect"
   - Selecione o `.bin` baixado
   - Clique em "Program"
3. Apos gravar, configure o WiFi (ver [Configuracao](#-configuracao))

### 🛠️ Opcao 2 — Compilar do Codigo Fonte

#### Pre-requisitos
- **Arduino IDE 2.x** ou **arduino-cli**
- **ESP32 Board Package 3.0+** (importante! v2.x nao funciona)
- Bibliotecas:
  - `FastLED` >= 3.6.0
  - `WebSockets` >= 2.4.1 (Markus Sattler)
  - `ArduinoJson` >= 7.0.0

#### Passos

```bash
# 1. Clone o repositorio
git clone https://github.com/Jean-DrEaD/rs50-thermal-controller.git
cd rs50-thermal-controller

# 2. Edite o config.h com suas credenciais
nano src/rs50_thermal/config.h

# 3. Abra src/rs50_thermal/rs50_thermal.ino na Arduino IDE
# 4. Selecione: Tools -> Board -> ESP32S3 Dev Module
# 5. Configure (ver tabela abaixo)
# 6. Upload!
```

---

## ⚙️ Configuracao

Edite **`src/rs50_thermal/config.h`** antes de gravar:

```cpp
// --- WIFI ---------------------------------------
#define WIFI_SSID         "SuaRedeAqui"        // <- seu WiFi 2.4GHz
#define WIFI_PASS         "SuaSenhaAqui"       // <- senha
#define WIFI_HOSTNAME     "rs50-thermal"       // <- http://rs50-thermal.local

// --- LEDs ---------------------------------------
#define LED_COUNT         3                    // <- 1, 3, 5+ LEDs
// LED_MODE selecionado automaticamente (1=SINGLE, 3-4=BAR, 5+=THERMOMETER)

// --- CALIBRACAO TERMICA -------------------------
#define THERMAL_OFFSET    9.0f                 // <- ajuste apos calibracao
#define LEAD_TIME_S       70.0f                // <- antecipacao em segundos
#define EMA_ALPHA         0.18f                // <- filtro de ruido

// --- LIMITES (NAO ALTERE sem motivo!) -----------
#define TEMP_WARNING      55.0f                // alerta amarelo
#define TEMP_CRITICAL     62.0f                // alerta vermelho
#define TEMP_SHUTDOWN     68.0f                // desliga motor
```

📖 **[Guia completo de calibracao -->](docs/calibration-guide.md)**

---

## 📊 Dashboard Web

Apos o boot, acesse no navegador:

```
http://rs50-thermal.local
```
ou pelo IP que aparecer no Serial Monitor.

### 🎨 Preview

```
+-------------------------------------------------+
|  🏎️  RS50 Thermal Controller          v3.4.0   |
|  ---------------------------------------------- |
|                                                 |
|        🌡️  Eixo:        47.2°C   [####  ]      |
|        🔥  Estator:     56.2°C   [##### ]  ⚠️   |
|        💨  Fan PWM:     78%      [####  ]      |
|        ⏱️  Sessao:      00:23:14                |
|        📈  Pico:        58.4°C                  |
|                                                 |
|        Estado: ⚠️ WARNING                       |
|        --------                                 |
|                                                 |
|        [Resetar Pico]   [Reiniciar]             |
+-------------------------------------------------+
```

> 📱 **Responsivo** — funciona em smartphone tambem

---

## 💡 Modos de LED

### `MODE_SINGLE` (1 LED)
Um unico LED indica o **estado geral**:
- 🟢 Verde — Normal
- 🟡 Amarelo — Warning
- 🔴 Vermelho pulsante — Critical
- 🔴 Vermelho fixo — Shutdown
- 🟣 Magenta — Fault

### `MODE_BAR` (3 LEDs)
- LED 1: Estado (cor do nivel atual)
- LED 2: Temperatura (gradiente azul->verde->amarelo->vermelho)
- LED 3: PWM da fan (gradiente cinza->branco)

### `MODE_THERMOMETER` (5+ LEDs)
LEDs preenchem como um termometro vertical:
- Cor do gradiente proporcional a temperatura
- Quantidade de LEDs acesos = nivel termico
- LED extra pisca em caso de FAULT

---

## 🎓 Como Funciona

### Estimativa adaptativa do estator

O NTC mede o **eixo do motor**, mas o ponto critico e o **estator**.
Existe um delta termico de ~9°C entre os dois.

```
T_estator_estimada = T_eixo + THERMAL_OFFSET + (dT/dt × LEAD_TIME)
                     |          |                |
                     |          |                +-- Antecipacao por inercia termica
                     |          +-- Diferenca estator/eixo (calibravel)
                     +-- Medicao direta do NTC
```

### Maquina de estados

```
       +----------+  T < 50°C    +----------+
       |  IDLE    |------------->|  NORMAL  |
       +----------+              +----+-----+
                                      | T > 55°C
                                      v
                                 +----------+
                                 | WARNING  |
                                 +----+-----+
                                      | T > 62°C
                                      v
                                 +----------+
                                 | CRITICAL |
                                 +----+-----+
                                      | T > 68°C
                                      v
                                 +----------+
       +----------+ T < 50°C     | SHUTDOWN |
       |  COOL    |<-------------| (relay)  |
       +----------+   + 60s      +----------+

       +----------+ NTC fail     +----------+
       | NORMAL   |------------->|  FAULT   |
       +----------+              +----------+
```

---

## 🌡️ Calibracao

A calibracao e **fortemente recomendada** apos a primeira instalacao.
Sem ela, o `THERMAL_OFFSET` default (9°C) pode estar otimista ou pessimista para sua montagem.

📖 **Veja o [Guia de Calibracao completo](docs/calibration-guide.md)** com:
- Procedimento passo-a-passo (~30 min)
- Folha de registro pronta
- Troubleshooting de problemas comuns
- Tabela de ajustes para cenarios tipicos

---

## 🛠️ Compilacao & Build Status

> ✅ **Status atual:** O sketch `src/rs50_thermal/rs50_thermal.ino` **compila sem erros nem warnings** na Arduino IDE 2.x com o core Arduino-ESP32 3.0+.

### Configuracao da placa (Arduino IDE)

| Setting | Valor |
|---------|-------|
| **Board** | ESP32S3 Dev Module |
| **USB CDC On Boot** | Enabled |
| **USB Mode** | Hardware CDC and JTAG |
| **Flash Size** | 4MB (32Mb) |
| **Partition Scheme** | Default 4MB with spiffs |
| **PSRAM** | Disabled (ESP32-S3-Zero) |
| **Upload Speed** | 921600 |

### Bibliotecas necessarias

```text
FastLED            >= 3.6.0
WebSockets         >= 2.4.1   (Markus Sattler)
ArduinoJson        >= 7.0.0
```

### Compilacao rapida

```bash
git clone https://github.com/Jean-DrEaD/rs50-thermal-controller.git
cd rs50-thermal-controller/src/rs50_thermal
nano config.h
# Abra rs50_thermal.ino na Arduino IDE -> Verify (Ctrl+R) -> Upload (Ctrl+U)
```

### Troubleshooting de upload

| Sintoma | Solucao |
|---------|---------|
| `Failed to connect to ESP32` | Segure **BOOT**, clique **RESET**, solte **BOOT** |
| `Sketch too big` | Mude particao para `Huge APP (3MB No OTA)` |
| `A fatal error occurred: MD5 mismatch` | Reduza Upload Speed para `460800` |
| LEDs nao acendem apos boot | Verifique `LED_COUNT` em `config.h` e 5V no WS2812 |

---

## 🤖 Para LLMs e Contribuidores Tecnicos

Este projeto tem um **briefing dedicado** para assistentes de IA e novos colaboradores que precisam entender rapidamente o estado do projeto:

📋 **[CONTEXT.md](CONTEXT.md)** — Estado atual, hardware definitivo, pinout real, decisoes arquivadas.

📐 **[docs/decisions/](docs/decisions/)** — ADRs (Architecture Decision Records) explicando porques tecnicos.

> 💡 Se voce e um chatbot/LLM ajudando neste projeto: leia `CONTEXT.md` **antes** de sugerir mudancas.

---

## 🤝 Contribuindo

Contribuicoes sao **muito bem-vindas**! 🎉

### Como contribuir:

1. 🍴 Faca um **fork** do projeto
2. 🌿 Crie uma branch: `git checkout -b feature/minha-feature`
3. 💾 Commit: `git commit -m 'feat: adiciona X'`
4. 📤 Push: `git push origin feature/minha-feature`
5. 🔀 Abra um **Pull Request**

### Tipos de contribuicao:

- 🐛 **Bug reports** ([template](.github/ISSUE_TEMPLATE/bug_report.md))
- ✨ **Feature requests** ([template](.github/ISSUE_TEMPLATE/feature_request.md))
- 📝 **Melhorias na documentacao** (sempre bem-vindas!)
- 🧪 **Reportar resultados de calibracao** (ajuda outros usuarios)
- 🌐 **Traducoes** (EN, ES, RU, JP...)
- 📸 **Fotos da sua montagem** (vai pra galeria!)

### Convencoes:

- **Commits**: [Conventional Commits](https://www.conventionalcommits.org/) (`feat:`, `fix:`, `docs:`, etc.)
- **Codigo**: 2 espacos, sem tabs (validado pela Action)
- **Versionamento**: [SemVer](https://semver.org/lang/pt-BR/)

---

## 📚 Documentacao

| Documento | Descricao |
|---|---|
| [📋 CONTEXT.md](CONTEXT.md) | Briefing tecnico (LLMs e novos contribuidores) |
| [📐 Wiring Schematic](docs/wiring-schematic.svg) | Diagrama eletrico completo |
| [🌡️ Calibration Guide](docs/calibration-guide.md) | Como calibrar o sistema |
| [📋 BOM CSV](hardware/bom.csv) | Lista de componentes |
| [📝 Changelog](CHANGELOG.md) | Historico de versoes |
| [🛡️ License](LICENSE) | MIT License + Disclaimer |
| [🏛️ Decisoes Arquiteturais](docs/decisions/) | ADRs do projeto |

---

## ⚠️ Disclaimer

> Este projeto controla **equipamento eletrico de alta potencia**. Instalacao
> incorreta pode causar **incendio**, **choque eletrico** ou **danos ao equipamento**.
>
> O autor **nao se responsabiliza** por danos resultantes do uso deste projeto.
> Use por sua conta e risco. Recomenda-se conhecimento basico de eletronica
> e respeito as normas de seguranca eletrica.
>
> **Sempre teste o sistema sem o motor ligado primeiro.**

---

## 🌟 Mostra de Suporte

Se este projeto te ajudou:

- ⭐ De uma **star** no repositorio!
- 🐦 Compartilhe nas redes sociais
- 💬 Conte sua experiencia nas [Discussions](https://github.com/Jean-DrEaD/rs50-thermal-controller/discussions)
- 📸 Envie fotos da sua montagem (vai pra galeria!)
- 🐛 Reporte bugs encontrados

---

## 👥 Comunidade

- 💬 **Discord FFBeast**: [#projects](https://discord.gg/ffbeast)
- 🐦 **Reddit**: [r/simracing](https://reddit.com/r/simracing)

---

## 📜 Licenca

Este projeto e licenciado sob a [**MIT License**](LICENSE) — sinta-se livre para usar, modificar e distribuir.

---

## 🙏 Agradecimentos

- 🏎️ **Comunidade FFBeast** pelas trocas tecnicas
- 🔧 **ODrive Robotics** pelo firmware open-source que inspirou o ODESC
- 💡 **Projetos open-source** que serviram de referencia:
  - [SimuCUBE](https://granitedevices.com/simucube)
  - [OpenFFBoard](https://openffboard.org/)
  - [BLDC Tool](https://github.com/vedderb/bldc)
- 👨‍💻 **AI Assistants** (Claude/Anthropic) que ajudaram com arquitetura e documentacao

---

<div align="center">

### Feito com ❤️ por [Jean-DrEaD](https://github.com/Jean-DrEaD)

**Para a comunidade Sim Racing brasileira e mundial** 🇧🇷🌍

[⬆ Voltar ao topo](#-rs50-thermal-controller)

</div>
