# tools/bump-version.ps1
# Uso: .\tools\bump-version.ps1 -NewVersion 3.4.0
param([Parameter(Mandatory)][string]$NewVersion)

if ($NewVersion -notmatch '^\d+\.\d+\.\d+$') {
    throw "Versão deve ser X.Y.Z"
}

# config.h
(Get-Content src/rs50_thermal/config.h) `
    -replace '(#define\s+FW_VERSION\s+")[^"]+(")', "`${1}$NewVersion`${2}" |
    Set-Content src/rs50_thermal/config.h

# .ino (linha de comentário)
(Get-Content src/rs50_thermal/rs50_thermal.ino) `
    -replace '(RS50 Thermal Controller v)\d+\.\d+\.\d+', "`${1}$NewVersion" |
    Set-Content src/rs50_thermal/rs50_thermal.ino

Write-Host "✅ Versão bumped para $NewVersion" -ForegroundColor Green
Write-Host "   Confira com: git diff" -ForegroundColor Yellow
