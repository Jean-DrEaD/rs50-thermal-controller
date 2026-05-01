# 🌡️ Guia de Calibração — RS50 Thermal Controller

> **Objetivo**: Ajustar o `THERMAL_OFFSET` para que a estimativa do estator
> corresponda à temperatura real medida com termômetro IR.

---

## 📋 Pré-requisitos

| Item | Necessidade |
|---|---|
| 🌡️ Termômetro infravermelho | Obrigatório (calibração) |
| 📱 Dispositivo com WiFi | Recomendado (acesso ao dashboard) |
| 🔌 Multímetro | Útil para validações |
| ⏱️ Cronômetro | Útil para sessões timed |
| 📊 Planilha (Excel/Sheets) | Recomendado para registrar dados |

---

## 🎯 Por que calibrar?

O sensor NTC está montado **no eixo do motor**, mas a temperatura crítica
acontece nas **bobinas do estator**. Existe um delta térmico entre os dois pontos:

---

# 🌡️ [Estator quente] ──(condução)──► [Eixo do motor] ──(NTC mede aqui)
# ~60-70°C ~50-60°C (zona crítica) (sempre menor)

O `THERMAL_OFFSET` (default: **9°C**) corrige essa diferença. Mas o valor real
depende de:
- 🔧 Acabamento da montagem do NTC (contato térmico)
- 🌬️ Posição da fan (fluxo de ar)
- 🏠 Ambiente (temperatura da sala)
- ⚙️ Padrão de uso (drift = mais calor que oval)

---

## 🚀 Procedimento de Calibração

### **Fase 1 — Sessão de Caracterização (~30 min)**

1. **Pré-aqueça o ambiente**: feche janelas, AC desligado se possível
   (queremos uma sessão "típica" do seu uso)

2. **Anote temperatura ambiente** com o IR apontado para uma parede:
# T_ambiente = ____°C

3. **Ligue o sistema** e aguarde 2 minutos de estabilização

4. **Registre baseline** (motor parado, sem uso):

5. **Inicie sessão de uso normal** (drift, ovais, ou jogo casual)
- Mantenha por **20 minutos contínuos**
- Não pause o motor

6. **A cada 5 minutos**, anote:

| Tempo | T_eixo (dash) | T_estator_est (dash) | T_eixo_IR | T_estator_IR | Δ real |
|---|---|---|---|---|---|
| 0min  | __ | __ | __ | __ | __ |
| 5min  | __ | __ | __ | __ | __ |
| 10min | __ | __ | __ | __ | __ |
| 15min | __ | __ | __ | __ | __ |
| 20min | __ | __ | __ | __ | __ |

> ⚠️ **Para medir com IR sem desligar**: faça uma **abertura pequena** na case
> próxima ao estator antes de começar. Não desligue o motor durante a sessão.

### **Fase 2 — Análise dos Dados**

1. **Calcule o delta médio real**:
# Δ_real = T_estator_IR - T_eixo_IR

2. **Calcule o delta médio das medições com carga**
(ignore os primeiros 5min — sistema ainda esquentando):
# Δ_médio = média(Δ_real para t ≥ 5min)

3. **Compare com o `THERMAL_OFFSET` atual** (default: 9°C):

| Δ_médio medido | Ação |
|---|---|
| 7-11°C | ✅ Default está bom, mantenha 9°C |
| 5-7°C | ⬇️ Reduza para 6-7°C (NTC bem acoplado) |
| 11-14°C | ⬆️ Aumente para 12-13°C (NTC mal acoplado) |
| < 5°C ou > 15°C | ⚠️ Verificar montagem do NTC! |

### **Fase 3 — Ajuste e Validação**

1. **Edite `config.h`**:
```cpp
#define THERMAL_OFFSET    11.0f   // ← novo valor
```
2. **Recompile e grave no ESP32**

3. **Repita Fase 1 com a nova configuração**

4. **Valide: agora a coluna T_estator_est (dash) deve estar dentro de ±2°C de T_estator_IR**

---