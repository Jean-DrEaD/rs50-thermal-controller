# schematic-data.ps1
# Dados de todos os componentes do schematic RS50

# Grid: 1.27mm (0.05 inch) - padrao KiCad
# Coordenadas em mm. Folha A4 paisagem: 297 x 210mm

$script:Components = @(
    # ============ BLOCO 1: POWER INPUT (X: 30-80) ============
    @{
        Ref = "J1"; Value = "XT60"; LibId = "rs50-custom:Conn_XT60_PCB_Mount"
        Footprint = "rs50-custom:Conn_XT60_PCB_Mount"
        X = 40; Y = 60; Rot = 0
        Labels = @{ "1" = "+24V"; "2" = "GND" }
    },
    @{
        Ref = "PS1"; Value = "Buck_24V_to_12V"; LibId = "rs50-custom:Buck_Module_HW411"
        Footprint = "rs50-custom:Buck_Module_HW411_4pin_P2.54mm"
        X = 80; Y = 60; Rot = 0
        Labels = @{ "1" = "+24V"; "2" = "GND"; "3" = "+12V"; "4" = "GND" }
    },
    @{
        Ref = "PS2"; Value = "Buck_12V_to_5V"; LibId = "rs50-custom:Buck_Module_HW411"
        Footprint = "rs50-custom:Buck_Module_HW411_4pin_P2.54mm"
        X = 80; Y = 90; Rot = 0
        Labels = @{ "1" = "+12V"; "2" = "GND"; "3" = "+5V"; "4" = "GND" }
    },

    # ============ BLOCO 2: ESP32-S3-ZERO (centro X: 130-170) ============
    @{
        Ref = "U1"; Value = "ESP32-S3-Zero"; LibId = "rs50-custom:ESP32-S3-Zero"
        Footprint = "rs50-custom:Module_ESP32-S3-Zero_2x11_Header"
        X = 150; Y = 110; Rot = 0
        Labels = @{
            "1"  = "+5V"
            "2"  = "GND"
            "3"  = "+3V3"
            "8"  = "NTC_ADC"      # GPIO4 (ADC1_CH3)
            "9"  = "FAN_PWM"      # GPIO5
            "10" = "RELAY_GATE"   # GPIO6
            "11" = "FAN_TACH"     # GPIO7
            "12" = "LED_DIN"      # GPIO8
        }
    },

    # ============ BLOCO 3: NTC + DIVISOR (X: 220-260, Y: 50-90) ============
    @{
        Ref = "R1"; Value = "100k"; LibId = "Device:R"
        Footprint = "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"
        X = 230; Y = 60; Rot = 0
        Labels = @{ "1" = "+3V3"; "2" = "NTC_ADC" }
    },
    @{
        Ref = "TH1"; Value = "NTC_100k_B3950"; LibId = "Device:Thermistor_NTC"
        Footprint = "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical"
        X = 230; Y = 80; Rot = 0
        Labels = @{ "1" = "NTC_ADC"; "2" = "GND" }
    },

    # ============ BLOCO 4: MOSFET + RELE + DIODO (X: 200-260, Y: 110-160) ============
    @{
        Ref = "R2"; Value = "220R"; LibId = "Device:R"
        Footprint = "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"
        X = 200; Y = 120; Rot = 90
        Labels = @{ "1" = "RELAY_GATE"; "2" = "MOSFET_G" }
    },
    @{
        Ref = "R3"; Value = "10k"; LibId = "Device:R"
        Footprint = "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"
        X = 210; Y = 130; Rot = 0
        Labels = @{ "1" = "MOSFET_G"; "2" = "GND" }
    },
    @{
        Ref = "Q1"; Value = "IRLZ44N"; LibId = "Transistor_FET:IRLZ44N"
        Footprint = "Package_TO_SOT_THT:TO-220-3_Vertical"
        X = 220; Y = 130; Rot = 0
        Labels = @{ "1" = "MOSFET_G"; "2" = "RELAY_COIL_LOW"; "3" = "GND" }
    },
    @{
        Ref = "K1"; Value = "SRD-24VDC-SL-C"; LibId = "Relay:SANYOU_SRD_Form_C"
        Footprint = "Relay_THT:Relay_SPDT_SANYOU_SRD_Series_Pitch5.0mm"
        X = 250; Y = 140; Rot = 0
        Labels = @{
            "1" = "+24V"          # coil +
            "2" = "RELAY_COIL_LOW" # coil -
            "3" = "+24V"          # COM (entrada)
            "4" = "MOTOR_OUT"     # NC (saida normal)
            "5" = ""              # NO (nao usado, opcional GND)
        }
    },
    @{
        Ref = "D1"; Value = "1N4007"; LibId = "Device:D"
        Footprint = "Diode_THT:D_DO-41_SOD81_P10.16mm_Horizontal"
        X = 240; Y = 140; Rot = 90
        Labels = @{ "1" = "RELAY_COIL_LOW"; "2" = "+24V" }
    },

    # ============ BLOCO 5: FAN PWM (X: 30-80, Y: 130-170) ============
    @{
        Ref = "J2"; Value = "FAN_4pin"; LibId = "Connector_Generic:Conn_01x04"
        Footprint = "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical"
        X = 50; Y = 150; Rot = 0
        Labels = @{
            "1" = "GND"        # preto
            "2" = "+12V"       # amarelo (fan 12V)
            "3" = "FAN_TACH"   # verde
            "4" = "FAN_PWM"    # azul
        }
    },

    # ============ BLOCO 6: WS2812 LEDs (X: 100-150, Y: 160-180) ============
    @{
        Ref = "LED1"; Value = "WS2812_Onboard"; LibId = "LED:WS2812B"
        Footprint = "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm"
        X = 110; Y = 170; Rot = 0
        Labels = @{
            "1" = "LED_DIN"
            "2" = "+5V"
            "3" = "LED_CHAIN"
            "4" = "GND"
        }
    },
    @{
        Ref = "J3"; Value = "LED_AUX_3pin"; LibId = "Connector_Generic:Conn_01x03"
        Footprint = "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical"
        X = 140; Y = 170; Rot = 0
        Labels = @{
            "1" = "+5V"
            "2" = "LED_CHAIN"
            "3" = "GND"
        }
    },

    # ============ BLOCO 7: MOTOR OUTPUT (X: 270-280) ============
    @{
        Ref = "J4"; Value = "MOTOR_OUT"; LibId = "Connector_Generic:Conn_01x02"
        Footprint = "TerminalBlock_Phoenix:TerminalBlock_Phoenix_MKDS-1,5-2-5.08_1x02_P5.08mm_Horizontal"
        X = 275; Y = 140; Rot = 0
        Labels = @{
            "1" = "MOTOR_OUT"
            "2" = "GND"
        }
    }
)

# Power flags (sao componentes especiais)
$script:PowerFlags = @(
    @{ Net = "+24V"; X = 40; Y = 50 },
    @{ Net = "+12V"; X = 95; Y = 55 },
    @{ Net = "+5V";  X = 95; Y = 85 },
    @{ Net = "+3V3"; X = 230; Y = 50 },
    @{ Net = "GND";  X = 40; Y = 75 }
)

Write-Host "schematic-data.ps1 carregado: $($script:Components.Count) componentes" -ForegroundColor Green
