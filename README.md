<div align="center">

# 🌡️ RS50 Thermal Controller

### Sistema inteligente de proteção térmica para volantes Direct Drive

[![Build Status](https://github.com/Jean-DrEaD/rs50-thermal-controller/actions/workflows/arduino-build.yml/badge.svg)](https://github.com/Jean-DrEaD/rs50-thermal-controller/actions/workflows/arduino-build.yml)
[![Release](https://img.shields.io/github/v/release/Jean-DrEaD/rs50-thermal-controller?color=brightgreen&label=release)](https://github.com/Jean-DrEaD/rs50-thermal-controller/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Made with Arduino](https://img.shields.io/badge/Made%20with-Arduino-00979D?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Sim Racing](https://img.shields.io/badge/community-Sim%20Racing-red)](https://www.reddit.com/r/simracing/)

**Proteção térmica fail-safe • Dashboard WiFi em tempo real • Estimativa adaptativa do estator**

[📥 Download](#-download--instalação) •
[🔧 Hardware](#-hardware) •
[📊 Dashboard](#-dashboard-web) •
[🌡️ Calibração](docs/calibration-guide.md) •
[🤝 Contribuir](#-contribuindo)

---

<img src="docs/banner.svg" alt="RS50 Thermal Controller" width="100%"/>

</div>

## 🎯 O Problema

Volantes **Direct Drive** com motores hoverboard 15Nm geram **calor extremo** durante sessões longas de drift, oval ou track. As bobinas do estator podem atingir **80°C+**, causando:

- 🔥 **Degradação do verniz** das bobinas (perda de isolação)
- 🧲 **Desmagnetização** dos ímãs N35 (perda permanente de torque)
- ⚡ **Saturação magnética** (sensação de "amolecimento" do FFB)
- 💥 **Falha catastrófica** em casos extremos

> **Soluções comerciais custam R$ 800+ e são caixas pretas.**  
> Este projeto resolve por **menos de R$ 150** com hardware off-the-shelf e firmware open-source.

---

## ✨ Features

<table>
<tr>
<td width="50%">

### 🛡️ Proteção
- ✅ **Relé fail-safe** (NC) — se o ESP travar, motor continua ligado
- ✅ **Estimativa adaptativa** do estator (com `THERMAL_OFFSET` calibrável)
- ✅ **Lead time** antecipa rampas térmicas (`dT/dt`)
- ✅ **3 níveis de proteção**: Warning → Critical → Shutdown
- ✅ **Latch de segurança** (60s) após desligamento

</td>
<td width="50%">

### 📊 Monitoramento
- ✅ **Dashboard WiFi** com WebSocket em tempo real
- ✅ **LEDs WS2812 configuráveis** (1, 3, 5+ LEDs)
- ✅ **Contador de horas** persistente em NVS
- ✅ **Pico de temperatura** da sessão
- ✅ **Detecção FAULT** com grace period adaptativo

</td>
</tr>
<tr>
<td width="50%">

### 🎛️ Controle
- ✅ **Curva PWM customizável** para fan 12V
- ✅ **Histerese** evita oscilação on/off
- ✅ **Filtro EMA** reduz ruído do ADC
- ✅ **3 modos de LED**: Single / Bar / Thermometer

</td>
<td width="50%">

### 🚀 DevOps
- ✅ **GitHub Actions** com build automático
- ✅ **Releases** com firmware `.bin` pronto
- ✅ **Templates** de Issue e PR
- ✅ **Semantic versioning** + Changelog
- ✅ **Documentação completa** em PT-BR

</td>
</tr>
</table>

---

## 🏗️ Arquitetura

```
┌─────────────────────────────────────────────────────────────┐
│                    Fonte 24V (XDrive Mini)                   │
└─────────────┬─────────────────────────────────┬─────────────┘
              │                                 │
              │ 24V                             │ 24V
              ▼                                 ▼
        ┌──────────┐                      ┌──────────┐
        │  Buck    │ 12V                  │  ODESC   │
        │ LM2596   │─────┬─► Fan 12V      │ FFBeast  │
        │ (24→12)  │     │                │          │
        └──────────┘     ▼                └─────┬────┘
              │      ┌───────────┐              │
              │ 12V  │ Buck Mini │ 5V           │ AC Out
              │      │ (12→5)    │──┐           │
              ▼      └───────────┘  │           ▼
        ┌──────────┐                │      ┌──────────┐
        │ Songle   │◄───── GPIO 4 ──┤      │  Motor   │
        │ Relé NC  │                │      │ Hoverbrd │
        │ 24VDC    │                │      │  15Nm    │
        └──────────┘                │      └─────┬────┘
                                    │            │
                              ┌─────▼─────┐      │
                              │ ESP32-S3  │      │
                              │   Zero    │      │
                              │           │      │
                              │  GPIO 1 ◄─┼──────┴── NTC 100K
                              │  GPIO 5 ──┼─► PWM Fan
                              │  GPIO 21 ─┼─► WS2812
                              │  GPIO 4 ──┼─► Relé
                              └───────────┘
```

📐 **[Diagrama detalhado em SVG](docs/wiring-schematic.svg)** • **[ASCII text](docs/wiring-ascii.txt)**

---

## 🔧 Hardware

### 📦 BOM (Bill of Materials)

| Qtd | Componente | Especificação | Preço aprox. |
|:---:|---|---|---:|
| 1 | **MCU** | Waveshare ESP32-S3-Zero | R$ 35 |
| 1 | **Sensor** | NTC 100K B3950 (vidro) | R$ 5 |
| 1 | **Relé** | Songle SLA-24VDC-SL-C (NC, 30A) | R$ 18 |
| 1 | **Buck principal** | LM2596 (24V→12V, 3A) | R$ 8 |
| 1 | **Buck lógica** | Mini-360 (12V→5V) | R$ 5 |
| 1 | **MOSFET driver** | IRLZ44N + resistor 220Ω | R$ 3 |
| 1 | **Fan** | 12V 80mm PWM 4-pin | R$ 25 |
| 1-5 | **LEDs** | WS2812B (avulso ou strip) | R$ 2-10 |
| 1 | **Resistor** | 100kΩ 1% (divisor NTC) | R$ 0.50 |
| 1 | **Capacitor** | 100nF cerâmico (filtro ADC) | R$ 0.50 |
| 1 | **Diodo flyback** | 1N4007 (proteção do relé) | R$ 0.50 |
| - | **Conectores** | JST-XH, terminais ring | R$ 10 |
| - | **Caixa** | Impressa 3D ou PVC | R$ 10 |
| | | **Total estimado** | **~R$ 145** |

📋 **[BOM completa em CSV](hardware/bom.csv)**

### 🎯 Compatibilidade

| Componente | Testado | Compatível |
|---|:---:|:---:|
| **RS50 Clone DD wheel** (XDrive Mini) | ✅ | ✅ |
| **Hoverboard motor 15Nm** | ✅ | ✅ |
| **ODESC FFBeast** | ✅ | ✅ |
| **ODrive v3.6** | ❌ | ⚠️ Provavelmente OK |
| **MaxxMotor (similar)** | ❌ | ⚠️ Adaptar conexões |
| **Outros DDs com fonte 24V** | ❌ | ⚠️ Adaptar Buck |

---

## 📥 Download & Instalação

### 🚀 Opção 1 — Firmware Pré-compilado (mais fácil)

1. Baixe a última `.bin` da [página de releases](https://github.com/Jean-DrEaD/rs50-thermal-controller/releases/latest)
2. Use o **[ESP Web Tools](https://espressif.github.io/esptool-js/)** no navegador (Chrome/Edge):
   - Conecte o ESP32-S3-Zero via USB-C
   - Clique em "Connect"
   - Selecione o `.bin` baixado
   - Clique em "Program"
3. Após gravar, configure o WiFi (ver [Configuração](#-configuração))

### 🛠️ Opção 2 — Compilar do Código Fonte

#### Pré-requisitos
- **Arduino IDE 2.x** ou **arduino-cli**
- **ESP32 Board Package 2.0.14+**
- Bibliotecas:
  - `FastLED` 3.6.0
  - `WebSockets` 2.4.1 (Markus Sattler)

#### Passos

```bash
# 1. Clone o repositório
git clone https://github.com/Jean-DrEaD/rs50-thermal-controller.git
cd rs50-thermal-controller

# 2. Edite o config.h com suas credenciais
# (use seu editor preferido)
nano src/rs50_thermal/config.h

# 3. Abra src/rs50_thermal/rs50_thermal.ino na Arduino IDE
# 4. Selecione: Tools → Board → ESP32S3 Dev Module
# 5. Configure:
#    - USB Mode: "Hardware CDC and JTAG"
#    - PSRAM: "OPI PSRAM"
#    - Flash Size: "4MB"
# 6. Upload!
```

---

## ⚙️ Configuração

Edite **`src/rs50_thermal/config.h`** antes de gravar:

```cpp
// ─── WIFI ──────────────────────────────────────────
#define WIFI_SSID         "SuaRedeAqui"        // ← seu WiFi 2.4GHz
#define WIFI_PASS         "SuaSenhaAqui"       // ← senha
#define WIFI_HOSTNAME     "rs50-thermal"       // ← acesso via http://rs50-thermal.local

// ─── LEDs ──────────────────────────────────────────
#define LED_COUNT         3                    // ← 1, 3, 5+ LEDs
// LED_MODE selecionado automaticamente por LED_COUNT (1=SINGLE, 3-4=BAR, 5+=THERMOMETER)

// ─── CALIBRAÇÃO TÉRMICA ───────────────────────────
#define THERMAL_OFFSET    9.0f                 // ← ajuste após calibração (ver guide)
#define LEAD_TIME_S       70.0f                // ← antecipação em segundos
#define EMA_ALPHA         0.18f                // ← filtro de ruído

// ─── LIMITES (NÃO ALTERE sem motivo!) ─────────────
#define TEMP_WARNING      55.0f                // alerta amarelo
#define TEMP_CRITICAL     62.0f                // alerta vermelho
#define TEMP_SHUTDOWN     68.0f                // desliga motor
```

📖 **[Guia completo de calibração →](docs/calibration-guide.md)**

---

## 📊 Dashboard Web

Após o boot, acesse no navegador:

```
http://rs50-thermal.local
```
ou pelo IP que aparecer no Serial Monitor.

### 🎨 Preview

```
┌─────────────────────────────────────────────────┐
│  🏎️  RS50 Thermal Controller          v3.3.0   │
│  ────────────────────────────────────────────   │
│                                                 │
│        🌡️  Eixo:        47.2°C   [████░░]      │
│        🔥  Estator:     56.2°C   [█████░]  ⚠️   │
│        💨  Fan PWM:     78%      [████░░]      │
│        ⏱️  Sessão:      00:23:14                │
│        📈  Pico:        58.4°C                  │
│                                                 │
│        Estado: ⚠️ WARNING                       │
│        ────────────────                         │
│                                                 │
│        [Resetar Pico]   [Reiniciar]             │
└─────────────────────────────────────────────────┘
```

> 📱 **Responsivo** — funciona em smartphone também

---

## 💡 Modos de LED

### `MODE_SINGLE` (1 LED)
Um único LED indica o **estado geral**:
- 🟢 Verde — Normal
- 🟡 Amarelo — Warning
- 🔴 Vermelho pulsante — Critical
- 🔴 Vermelho fixo — Shutdown
- 🟣 Magenta — Fault

### `MODE_BAR` (3 LEDs)
- LED 1: Estado (cor do nível atual)
- LED 2: Temperatura (gradiente azul→verde→amarelo→vermelho)
- LED 3: PWM da fan (gradiente cinza→branco)

### `MODE_THERMOMETER` (5+ LEDs)
LEDs preenchem como um termômetro vertical:
- Cor do gradiente proporcional à temperatura
- Quantidade de LEDs acesos = nível térmico
- LED extra pisca em caso de FAULT

---

## 🎓 Como Funciona

### Estimativa adaptativa do estator

O NTC mede o **eixo do motor**, mas o ponto crítico é o **estator**.  
Existe um delta térmico de ~9°C entre os dois.

```
T_estator_estimada = T_eixo + THERMAL_OFFSET + (dT/dt × LEAD_TIME)
                     ↑          ↑                ↑
                     |          |                └─ Antecipação por inércia térmica
                     |          └─ Diferença estator/eixo (calibrável)
                     └─ Medição direta do NTC
```

### Máquina de estados

```
       ┌──────────┐  T < 50°C    ┌──────────┐
       │  IDLE    │─────────────►│  NORMAL  │
       └──────────┘              └────┬─────┘
                                      │ T > 55°C
                                      ▼
                                 ┌──────────┐
                                 │ WARNING  │
                                 └────┬─────┘
                                      │ T > 62°C
                                      ▼
                                 ┌──────────┐
                                 │ CRITICAL │
                                 └────┬─────┘
                                      │ T > 68°C
                                      ▼
                                 ┌──────────┐
       ┌──────────┐ T < 50°C     │ SHUTDOWN │
       │  COOL    │◄─────────────│ (relay)  │
       └──────────┘   + 60s      └──────────┘
                                      
       ┌──────────┐ NTC fail     ┌──────────┐
       │ NORMAL   │─────────────►│  FAULT   │
       └──────────┘              └──────────┘
```

---

## 🌡️ Calibração

A calibração é **fortemente recomendada** após a primeira instalação.  
Sem ela, o `THERMAL_OFFSET` default (9°C) pode estar otimista ou pessimista para sua montagem.

📖 **Veja o [Guia de Calibração completo](docs/calibration-guide.md)** com:
- Procedimento passo-a-passo (~30 min)
- Folha de registro pronta
- Troubleshooting de problemas comuns
- Tabela de ajustes para cenários típicos

---

## 🤝 Contribuindo

Contribuições são **muito bem-vindas**! 🎉

### Como contribuir:

1. 🍴 Faça um **fork** do projeto
2. 🌿 Crie uma branch: `git checkout -b feature/minha-feature`
3. 💾 Commit: `git commit -m 'feat: adiciona X'`
4. 📤 Push: `git push origin feature/minha-feature`
5. 🔀 Abra um **Pull Request**

### Tipos de contribuição:

- 🐛 **Bug reports** ([template](.github/ISSUE_TEMPLATE/bug_report.md))
- ✨ **Feature requests** ([template](.github/ISSUE_TEMPLATE/feature_request.md))
- 📝 **Melhorias na documentação** (sempre bem-vindas!)
- 🧪 **Reportar resultados de calibração** (ajuda outros usuários)
- 🌐 **Traduções** (EN, ES, RU, JP...)
- 📸 **Fotos da sua montagem** (vai pra galeria!)

### Convenções:

- **Commits**: [Conventional Commits](https://www.conventionalcommits.org/) (`feat:`, `fix:`, `docs:`, etc.)
- **Código**: 2 espaços, sem tabs (validado pela Action)
- **Versionamento**: [SemVer](https://semver.org/lang/pt-BR/)

---

## 🗺️ Roadmap

### v3.4 — *Próximo release*
- [ ] OTA Updates via WiFi
- [ ] Configuração via dashboard (sem recompilar)
- [ ] Histórico CSV em SPIFFS
- [ ] API REST `/api/status`, `/api/config`

### v3.5
- [ ] MQTT integration (Home Assistant)
- [ ] Notificações push (Telegram/Discord webhook)
- [ ] Modo "treino" (perfis de uso)

### v4.0 — *Long term*
- [ ] PCB customizada (KiCad)
- [ ] Versão "Pro" com sensor extra no ODESC
- [ ] App mobile dedicado
- [ ] Suporte a outros motores (MaxxMotor, simucube clones)

📋 **[Veja todas as issues abertas →](https://github.com/Jean-DrEaD/rs50-thermal-controller/issues)**

---

## 📚 Documentação

| Documento | Descrição |
|---|---|
| [📐 Wiring Schematic](docs/wiring-schematic.svg) | Diagrama elétrico completo |
| [🌡️ Calibration Guide](docs/calibration-guide.md) | Como calibrar o sistema |
| [📋 BOM CSV](hardware/bom.csv) | Lista de componentes |
| [📝 Changelog](CHANGELOG.md) | Histórico de versões |
| [🛡️ License](LICENSE) | MIT License + Disclaimer |

---

## ⚠️ Disclaimer

> Este projeto controla **equipamento elétrico de alta potência**. Instalação 
> incorreta pode causar **incêndio**, **choque elétrico** ou **danos ao equipamento**.
> 
> O autor **não se responsabiliza** por danos resultantes do uso deste projeto.  
> Use por sua conta e risco. Recomenda-se conhecimento básico de eletrônica  
> e respeito às normas de segurança elétrica.
> 
> **Sempre teste o sistema sem o motor ligado primeiro.**

---

## 🌟 Mostra de Suporte

Se este projeto te ajudou:

- ⭐ Dê uma **star** no repositório!
- 🐦 Compartilhe nas redes sociais
- 💬 Conte sua experiência nas [Discussions](https://github.com/Jean-DrEaD/rs50-thermal-controller/discussions)
- 📸 Envie fotos da sua montagem (vai pra galeria!)
- 🐛 Reporte bugs encontrados

---

## 👥 Comunidade

- 💬 **Discord FFBeast**: [#projects](https://discord.gg/ffbeast)
- 🐦 **Reddit**: [r/simracing](https://reddit.com/r/simracing)

---

## 📜 Licença

Este projeto é licenciado sob a [**MIT License**](LICENSE) — sinta-se livre para usar, modificar e distribuir.

---

## 🙏 Agradecimentos

- 🏎️ **Comunidade FFBeast** pelas trocas técnicas
- 🔧 **ODrive Robotics** pelo firmware open-source que inspirou o ODESC
- 💡 **Projetos open-source** que serviram de referência:
  - [SimuCUBE](https://granitedevices.com/simucube)
  - [OpenFFBoard](https://openffboard.org/)
  - [BLDC Tool](https://github.com/vedderb/bldc)
- 👨‍💻 **AI Assistants** (Claude/Anthropic) que ajudaram com arquitetura e documentação

---

<div align="center">

### Feito com ❤️ por [Jean-DrEaD](https://github.com/Jean-DrEaD)

**Para a comunidade Sim Racing brasileira e mundial** 🇧🇷🌍

[⬆ Voltar ao topo](#-rs50-thermal-controller)

</div>
