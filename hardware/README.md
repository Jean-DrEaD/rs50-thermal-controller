# RS50 Thermal Controller - Hardware

> PCB design em KiCad 10 para a placa controladora termica do volante DD RS50 Clone.

[![KiCad 10](https://img.shields.io/badge/KiCad-10.0%2B-blue)](https://www.kicad.org/)
[![Board Rev](https://img.shields.io/badge/PCB-v0.1--draft-orange)]()

---

## Como abrir

1. Instale o [KiCad 10.0+](https://www.kicad.org/download/)
2. Checkout da branch `hardware/v0.1`:
   ```bash
   git checkout hardware/v0.1
   ```
3. Abra `hardware/rs50-thermal.kicad_pro` no KiCad

---

## Estrutura

```
hardware/
|-- rs50-thermal.kicad_pro     # Project file
|-- rs50-thermal.kicad_sch     # Schematic raiz (etapa 4)
|-- rs50-thermal.kicad_pcb     # PCB layout (etapa 5)
|-- sheets/                    # Sub-schematics hierarquicos
|-- symbols/                   # Simbolos custom
|-- footprints/                # .pretty libs custom
|-- 3dmodels/                  # STEP/WRL para render 3D
|-- docs/                      # Diagramas, calculations
`-- production/                # Gerber/drill/BOM (gitignored)
```

---

## Especificacoes Eletricas

| Parametro          | Valor                                    |
|--------------------|------------------------------------------|
| Entrada            | 24 VDC (PSU 30A)                         |
| Saidas reguladas   | 12V/3A (fan), 5V/3A (logica)             |
| Carga chaveada     | 24VDC ate 30A via rele NC                |
| Consumo logica     | < 200 mA @ 5V                            |
| Dimensoes PCB      | 80 x 60 mm                               |
| Camadas            | 2-layer FR4 1.6mm                        |

Nota: XDrive Mini e ODESC FFBeast referem-se ao MESMO driver BLDC
(motor hoverboard 15Nm). O rele chaveia o 24VDC entre a PSU e este driver.

---

## Net Classes

| Classe      | Track   | Via         | Cor       | Uso                |
|-------------|:-------:|:-----------:|:---------:|--------------------|
| Power_24V   | 2.0 mm  | 1.2/0.6 mm  | Vermelho  | +24V principal     |
| Power_12V   | 1.0 mm  | 1.0/0.5 mm  | Laranja   | +12V (fan)         |
| Power_5V    | 0.5 mm  | 0.8/0.4 mm  | Verde     | +5V, +3V3 logica   |
| Signal      | 0.25 mm | 0.6/0.3 mm  | Azul      | GPIOs, ADC, PWM    |

---

## Filosofia Fail-Safe

3 camadas de protecao:

1. **Rele NC** - Se ESP32 desligar, motor continua LIGADO. Apenas falhas
   termicas explicitas (T > TEMP_SHUTDOWN) cortam a alimentacao.
2. **Pull-down 10k no Gate** - MOSFET fica OFF se GPIO flutuar no boot.
3. **Diodo flyback 1N4007** - Protege MOSFET do spike indutivo da bobina.

---

## BOM

Ver [`../BOM.csv`](../BOM.csv) na raiz do repo.

Custo estimado: ~R$ 145 (componentes) + ~R$ 80 (PCB JLCPCB 5un).

---

## Revisoes

| Rev    | Data       | Mudancas                              |
|:------:|------------|---------------------------------------|
| v0.1   | 2026-05-01 | Initial draft (project + net classes) |

---

## Licenca

Hardware sob CERN-OHL-S v2 (compativel com MIT do firmware).