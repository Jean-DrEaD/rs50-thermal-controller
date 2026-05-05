# 🧠 Contexto do Projeto (para LLMs e colaboradores)

> **Leia isto antes de fazer qualquer alteracao significativa.**
> Este arquivo da o estado atual do projeto sem precisar revisitar todo o historico.

## 📍 Estado Atual (2026-05-05)

- **Branch ativa**: `hardware/v0.1`
- **Versao README**: v3.3.0 (sera atualizada para v3.4 em breve)
- **Hardware design tool**: Fritzing 1.0.6 (`.fzz` em `hardware/`)
- **Schematic KiCad**: ❌ ABANDONADO (ver ADR-001)

## 🎯 Hardware Definitivo (decisoes finais)

| Componente | Modelo | Funcao |
|------------|--------|--------|
| MCU | Waveshare ESP32-S3-Zero | Logica de controle |
| Sensor | NTC 100K B3950 | Temperatura do eixo |
| Rele | **Songle SLA-24VDC-SL-C** (SPDT, 30A) | Fail-safe motor |
| Buck principal | LM2596 HW-411 (24V→12V) | Alimenta fan |
| Buck logica | LM2596 HW-411 (12V→5V) | Alimenta ESP32 |
| MOSFET | IRLZ44N + R 220Ω | Driver da bobina do rele |
| Fan | 12V 80mm PWM 4-pin | Cooling ativo |
| Diodo | 1N4007 | Flyback da bobina |
| Conector entrada | XT60 | Da fonte 24V do XDrive Mini |

## 🔌 Pinout ESP32-S3-Zero (definitivo)

| GPIO | Funcao | Direcao |
|------|--------|---------|
| GPIO1 | NTC ADC + filtro 100nF | IN (analog) |
| GPIO2 | PWM Fan | OUT |
| GPIO3 | WS2812 DIN | OUT |
| GPIO4 | Gate IRLZ44N (rele) | OUT |
| GPIO5 | TACH Fan (opcional) | IN (digital, pull-up) |

## 🛡️ Filosofia Fail-Safe

Rele em modo **NC (Normally Closed)**: bobina desenergizada = motor LIGADO.
Apenas em estado SHUTDOWN o ESP energiza a bobina, **abrindo** o circuito
e desligando o motor. Se o ESP travar/queimar, motor permanece operacional
(falha segura para o usuario que esta dirigindo).

## 📝 Decisoes Arquivadas

- [ADR-001](docs/decisions/ADR-001-abandona-kicad.md): Abandono do KiCad

## 🤖 Para LLMs/Chatbots

Quando retomar trabalho neste repo:
1. Leia este `CONTEXT.md` primeiro
2. Veja `docs/decisions/` para entender porques
3. README.md tem visao publica; este arquivo tem visao interna
4. Branch ativa pode mudar - confira com `git branch --show-current`