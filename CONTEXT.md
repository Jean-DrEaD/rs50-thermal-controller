# Contexto do Projeto (para LLMs e colaboradores)

> **Leia isto antes de fazer qualquer alteracao significativa.**
> Este arquivo da o estado atual do projeto sem precisar revisitar todo o historico.

## Estado Atual (2026-05-05)

- **Branch ativa**: `hardware/v0.1`
- **Versao firmware**: v3.4.0 (alinhado com README)
- **Hardware design tool**: Fritzing 1.0.6 (`.fzz` em `hardware/`)
- **Schematic KiCad**: ABANDONADO (ver ADR-001)

## Hardware Definitivo

| Componente | Modelo | Funcao |
|------------|--------|--------|
| MCU | Waveshare ESP32-S3-Zero | Logica de controle |
| Sensor | NTC 100K B3950 | Temperatura do eixo |
| Rele | Songle SLA-24VDC-SL-C (SPDT, 30A) | Fail-safe motor |
| Buck #1 | LM2596 HW-411 (24V->12V, 3A) | Alimenta fan + bobina rele |
| Buck #2 | LM2596 HW-411 (12V->5V, 1A) | Alimenta ESP32-S3-Zero |
| MOSFET | IRLZ44N + 220R + 10K pulldown | Driver da bobina do rele |
| Fan | 12V 120mm PWM 4-pin | Cooling ativo (Intel-style 25kHz) |
| Diodo | 1N4007 | Flyback da bobina do rele |
| Conector entrada | XT60 femea PCB-mount | Fonte 24V (cabo c/ macho) |
| LED | WS2812B onboard + opcional strip | Indicador visual de estado |

## Pinout ESP32-S3-Zero (DEFINITIVO - extraido de config.h)

| GPIO | Constante | Funcao | Direcao | Detalhe |
|------|-----------|--------|---------|---------|
| GPIO1 | `PIN_NTC` | NTC ADC1_CH0 | IN (analog) | Divisor c/ 100K + filtro 100nF |
| GPIO4 | `PIN_RELAY` | Gate MOSFET (rele) | OUT | Via R 220R + pulldown 10K |
| GPIO5 | `PIN_PWM` | Fan PWM (azul) | OUT | 25kHz, 8 bits, min 40% |
| GPIO21 | `PIN_LED_RGB` | WS2812 DIN | OUT | LED onboard + strip externa |

> **IMPORTANTE**: Cabo verde do fan (TACH) NAO e usado. Cortar e isolar.

## Constantes-Chave (config.h)

```cpp
#define PWM_FREQ              25000   // 25 kHz (acima do audivel)
#define PWM_RESOLUTION        8       // 8 bits (0-255)
#define PWM_MIN               40      // % minimo (sempre tem fluxo)
#define PWM_MAX               100
#define NTC_BETA              3950.0f
#define RELAY_FAILSAFE_NC     true    // Bobina OFF = motor ON
#define LED_COUNT             1       // Ajustavel: 1/3/5+
#define LED_COLOR_ORDER       GRB     // Waveshare ESP32-S3-Zero
```

## Filosofia Fail-Safe

Rele em modo NC (Normally Closed): bobina desenergizada = motor LIGADO.
Apenas em estado SHUTDOWN o ESP energiza a bobina (via MOSFET no GPIO4),
abrindo o circuito e desligando o motor. Se o ESP travar/queimar, motor
permanece operacional (falha segura para o usuario que esta dirigindo).

## Fluxo de Energia

```
Fonte 24V (XDrive Mini)
    |
    +-- XT60 macho (chicote) -> XT60 femea (PCB)
    |
    +-- HW-411 #1 (24V -> 12V, 3A)
    |       +-- Fan 120mm (amarelo)
    |       +-- Bobina rele (via MOSFET IRLZ44N)
    |
    +-- HW-411 #2 (12V -> 5V, 1A)  [cascata do #1]
            +-- ESP32-S3-Zero (5V pin)
                    +-- GPIO1 <- NTC (eixo motor)
                    +-- GPIO4 -> MOSFET -> Bobina rele
                    +-- GPIO5 -> Fan PWM
                    +-- GPIO21 -> WS2812
```

## Decisoes Arquivadas

- ADR-001 (`docs/decisions/ADR-001-abandona-kicad.md`): Abandono do KiCad

## Para LLMs/Chatbots

Quando retomar trabalho neste repo:
1. Leia este `CONTEXT.md` primeiro
2. Veja `docs/decisions/` para entender porques
3. README.md tem visao publica; este arquivo tem visao interna
4. Branch ativa pode mudar - confira com `git branch --show-current`
5. **Pinout REAL esta em `src/rs50_thermal/config.h`** (fonte da verdade)
