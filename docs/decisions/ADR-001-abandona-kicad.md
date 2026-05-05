# ADR-001: Abandono do KiCad em favor do Fritzing

**Data**: 2026-05-05
**Status**: ✅ Aceito
**Contexto**: hardware/v0.1 branch

## Contexto

O projeto teve fase exploratoria com KiCad para schematic capture, mas:

1. **Objetivo do projeto e montagem manual** (protoboard/perfboard), nao fabricacao de PCB
2. KiCad gera trabalho duplicado: o mesmo circuito precisa ser desenhado
   no Fritzing tambem para servir de guia de montagem visual
3. Ferramentas de geracao automatica (scripts PowerShell) consumiram
   tempo desproporcional ao valor entregue
4. Comunidade alvo (sim racers DIY) consome melhor diagramas Fritzing

## Decisao

- ❌ **Abandonar KiCad** para versoes 3.x
- ✅ **Adotar Fritzing** como fonte oficial de hardware design
- 📦 Arquivos `.kicad_*` movidos para `hardware/legacy/` (preserva historico Git)
- 🚫 `.gitignore` atualizado para ignorar artefatos KiCad futuros

## Quando Reverter

Se v4.0 (roadmap) for implementada com **PCB customizada fabricada**,
KiCad volta a ser ferramenta oficial. Ate la, Fritzing reina.

## Consequencias

✅ Positivas:
- Documentacao unica (Fritzing + SVG export)
- Onboarding mais rapido para contribuidores
- Foco no firmware, que e o core do projeto

⚠️ Negativas:
- Sem netlist formal para validacao automatizada
- ERC (Electrical Rules Check) do KiCad perdido
- Migracao para PCB no futuro exigira recriacao

## Referencias

- Fritzing oficial: https://fritzing.org
- Discussao no chat: 2026-05-05 (Claude Opus 4.7)