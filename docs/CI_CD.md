# CI/CD — RS50 Thermal Controller

## Workflows

### `build.yml` — Arduino Build & Validate
**Disparo:** push em `main`/`develop` (paths `src/**`, `ci/**`), PRs para `main`, manual.

Etapas:
1. Instala Arduino CLI + core `esp32:esp32@2.0.14`
2. Lê dependências de `ci/libraries.txt`
3. Compila `src/rs50_thermal/` com FQBN ESP32-S3-Zero (sem PSRAM)
4. Verifica tamanho do binário (warning >1.25MB)
5. Publica artefato `rs50-thermal-firmware-<sha>` (retenção 30d)
6. Job `lint`: tabs, trailing whitespace, padrão `FW_VERSION 3.3.x`

### `release.yml` — Release Build
**Disparo:** push de tag `v*.*.*`.

Etapas:
1. **Valida que a tag bate com `FW_VERSION` em `config.h`** (falha se divergir)
2. Instala core + libs de `ci/libraries.txt`
3. Compila com FQBN **idêntico** ao build
4. Renomeia artefatos para `rs50_thermal_v<versão>{,_bootloader,_partitions}.bin`
5. Publica GitHub Release com os 3 binários e instruções de gravação

## Fonte única de dependências: `ci/libraries.txt`
Formato `Nome@versão`, uma por linha. Sincronizar com o ambiente local.

Atualmente:
- `FastLED@3.6.0`
- `WebSockets@2.4.1`

## FQBN oficial
esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashMode=qio,FlashSize=4M,PartitionScheme=default,PSRAM=disabled,LoopCore=1,EventsCore=1,DebugLevel=none

## Como liberar uma versão (passo a passo)
1. Atualizar `FW_VERSION` em `src/rs50_thermal/config.h` (ex.: `"3.3.4"`)
2. Atualizar `CHANGELOG.md`
3. Commit + push em `main`
4. Aguardar build verde
5. Criar tag **igual** ao `FW_VERSION` com prefixo `v`:
   ```
   bash
   git tag v3.3.4
   git push origin v3.3.4
   ```
6. O release.yml valida, compila e publica o Release automaticamente.
⚠️ Se a tag não bater com FW_VERSION, o release falha de propósito.

---

## 🚀 5) Comandos para commit (PowerShell, Windows)

> ⚠️ **Antes de rodar**: confira a versão atual em `src/rs50_thermal/config.h`. Vou assumir **`3.3.4`** como exemplo — **ajuste** se for outra.

```powershell
# 1) Criar pasta e arquivos
New-Item -ItemType Directory -Force -Path ci, docs | Out-Null

# 2) ci/libraries.txt
@"
# Bibliotecas externas do projeto RS50 Thermal Controller
# Formato: Nome@versão (uma por linha). Linhas iniciadas com '#' são comentários.
FastLED@3.6.0
WebSockets@2.4.1
"@ | Set-Content -Path ci\libraries.txt -Encoding UTF8

# 3) Salvar os .yml revisados (copie o conteúdo das seções acima para os arquivos)
#    .github\workflows\build.yml
#    .github\workflows\release.yml
#    docs\CI_CD.md

# 4) Confirmar versão atual ANTES de taggear
Select-String -Path src\rs50_thermal\config.h -Pattern 'FW_VERSION'

# 5) Commit das mudanças de CI
git add ci/libraries.txt `
        .github/workflows/build.yml `
        .github/workflows/release.yml `
        docs/CI_CD.md
git commit -m "ci: unifica deps em ci/libraries.txt, alinha FQBN build/release e valida tag vs FW_VERSION"
git push origin main

# 6) AGUARDAR o build do main ficar verde no GitHub Actions

# 7) Criar tag SOMENTE depois do build verde
#    Substitua 3.3.4 pela versão real de FW_VERSION em config.h
$VERSION = "3.3.4"
git tag "v$VERSION"
git push origin "v$VERSION"
