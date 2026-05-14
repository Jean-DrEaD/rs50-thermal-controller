# RS50 Thermal Controller — Projeto

> **Versão atual:** 3.3.13
> **Última atualização:** 2026-05-14
> Documento de referência estável do projeto. Para o estado da sessão atual, ver [`CONTEXT.md`](./CONTEXT.md).

---

## 💡 Resumo

Controle térmico **fail-safe** para o RS50, baseado em ESP32-S3, monitorando a temperatura do estator do motor BLDC 15 Nm acionado pelo **MKS XDrive Mini (Firmware FFBeast)**. O sistema gerencia ventoinha PWM, indicação visual via WS2812 e corte de potência por relé NC em caso de sobreaquecimento.

---

## 🔧 Hardware (BOM)

| Qtde | Componente                  | Notas                              |
|------|-----------------------------|------------------------------------|
| 1    | ESP32-S3-Zero (Waveshare)   | MCU principal, sem PSRAM           |
| 1    | Relé Songle SLA-24VDC-SL-C  | NC, SPDT, 30 A                     |
| 1    | MOSFET IRLZ44N              | Logic-level, aciona bobina do relé |
| 1    | Diodo 1N4007                | Flyback na bobina                  |
| 2    | Reguladores LM2596          | 24→12 V e 12→5 V                   |
| 1    | NTC 100k B3950              | Encapsulado em vidro, no estator   |
| 1    | Fan 120 mm PWM              | 25 kHz, sinal 3.3 V direto         |
| 1    | Fita WS2812 (5 LEDs)        | Em série, resistor 330 Ω no DIN    |
| —    | Resistores                  | 220 Ω, 10 k, 100 k, 330 Ω          |
| —    | Capacitor 100 nF            | Filtro no divisor do NTC           |
| 1    | Conector XT60               | Entrada principal 24 V             |

---

## 🛡️ Topologia Fail-safe

- **+24 V → Relé NC → MKS VIN+**
- **GND** vai direto da fonte ao MKS (não passa pelo relé)
- Sem 5 V na ESP → relé permanece **fechado** → motor continua operando
- `GPIO4 = HIGH` → MOSFET energiza bobina → relé **abre** → motor para
- Durante **OTA**: `fan = 0` e `relay = LOW` (LEDs em magenta como indicação visual)

---

## 📌 Pinagem ESP32-S3

| GPIO | Função          | Macro         | Notas                            |
|------|-----------------|---------------|----------------------------------|
| 1    | NTC ADC         | `PIN_NTC`     | Divisor 100 k + cap 100 nF       |
| 4    | Gate MOSFET     | `PIN_RELAY`   | 220 Ω em série + pull-down 10 k  |
| 5    | PWM Fan         | `PIN_PWM`     | 25 kHz, 3.3 V direto             |
| 9    | WS2812 Data     | `PIN_WS2812`  | 330 Ω em série, 5 LEDs           |
| 21   | ⚠️ Não usar     | —             | LED onboard da placa             |

---

## 🌡️ FSM de Temperatura

| Faixa         | Estado     | Ação                              | Macro            |
|---------------|------------|-----------------------------------|------------------|
| < 40 °C       | IDLE       | Fan OFF, LED azul                 | `TEMP_IDLE`      |
| 40–60 °C      | WARMING    | PWM proporcional, LED verde       | `TEMP_WARMING`   |
| 60–68 °C      | WARNING    | PWM 100 %, LED amarelo            | `TEMP_WARNING`   |
| ≥ 68 °C       | CRITICAL   | Motor cortado (relé abre), vermelho | `TEMP_CRITICAL` |
| < 63 °C (pós) | RESTART    | Religa motor (histerese 5 °C)     | `TEMP_RESTART`   |

---

## 🏗️ Stack & Build

- **Arduino core:** ESP32 v2.0.14 (fixado)
- **FQBN:** `esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashMode=qio,FlashSize=4M,PartitionScheme=default,PSRAM=disabled`
- **Sketch:** `src/rs50_thermal/rs50_thermal.ino`
- **Config local:** `src/rs50_thermal/config.h` (ignorado no git; use `config.h.example`)
- **Macro de versão:** `FW_VERSION` no formato `3.3.x`

---

### Comandos principais

```
# Compilar
arduino-cli compile --fqbn esp32:esp32:esp32s3 src/rs50_thermal

# Flash (modo download via GPIO0 + RESET)
arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32s3 src/rs50_thermal

# Monitor serial
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

---

### 🤖 CI / Automação

- **Workflow principal:** build + lint a cada push/PR
- **Release:** push de tag v3.3.x → build + release automático no GitHub
- **Banner:** banner-bump.yml atualiza banner.svg?v=... no push da tag
- **Pre-commit hook:** .githooks/pre-commit valida FW_VERSION, header e config.h

# Ativar hooks localmente:

```
git config core.hooksPath .githooks
```
---

### 🧪 Processo de Bring-up

1. Carga — verificar 24 V, 12 V e 5 V nos pontos esperados
2. Boot — flash via GPIO0+RESET, validar CDC serial
3. Sensores — leitura estável do NTC (filtro 100 nF presente)
4. PWM — fan resposta linear em 25 kHz
5. Relé — acionamento por GPIO4, confirmar abertura do NC
6. Teste de motor — partida controlada com MKS conectado
7. OTA — validar entrada em modo seguro (fan=0, relay=LOW, magenta)

---

### 📂 Estrutura do repositório

```
.
├── .githooks/
│   └── pre-commit
├── .github/workflows/
│   ├── build.yml
│   └── banner-bump.yml
├── docs/
│   └── ...
├── src/rs50_thermal/
│   ├── rs50_thermal.ino
│   ├── config.h.example
│   └── config.h            # gitignored
├── banner.svg
├── PROJECT.md
├── CONTEXT.md
└── README.md

```
---

### 📝 Notas históricas relevantes

- **FW_VERSION fixada em 3.3.13 (fix do UDP_HOST no .example)**
- **Arduino core ESP32 fixado em 2.0.14 para reprodutibilidade**
- **Banner com cache-buster automatizado via workflow no push da tag**
- **Documentação alinhada ao hardware físico montado e validado**
