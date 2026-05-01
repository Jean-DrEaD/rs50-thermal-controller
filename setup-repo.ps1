# ════════════════════════════════════════════════════════════════
#  RS50 Thermal Controller — Setup PowerShell
#  Versão: v3.3.0
#  Uso: .\setup-repo.ps1
# ════════════════════════════════════════════════════════════════

# ─── CONFIGURAÇÃO ───────────────────────────────────────────────
$REPO_NAME    = "rs50-thermal-controller"
$GITHUB_USER  = "Jean-DrEaD"
$VERSION      = "v3.3.0"
$REMOTE_URL   = "https://github.com/$GITHUB_USER/$REPO_NAME.git"

Write-Host ""
Write-Host "╔══════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  RS50 Thermal Controller — Setup do Repositório  ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# ─── PASSO 1: Verificar identidade Git ──────────────────────────
Write-Host "🔍 Verificando configuração do Git..." -ForegroundColor Yellow
$gitName  = git config --global user.name
$gitEmail = git config --global user.email

if (-not $gitName -or -not $gitEmail) {
    Write-Host "❌ Identidade Git não configurada!" -ForegroundColor Red
    Write-Host "   Execute:" -ForegroundColor Yellow
    Write-Host '   git config --global user.name "Jean-DrEaD"' -ForegroundColor White
    Write-Host '   git config --global user.email "seu_email@exemplo.com"' -ForegroundColor White
    exit 1
}
Write-Host "✅ Identidade: $gitName <$gitEmail>" -ForegroundColor Green

# ─── PASSO 2: Criar estrutura de pastas ─────────────────────────
Write-Host ""
Write-Host "📂 Criando estrutura de pastas..." -ForegroundColor Yellow

$folders = @(
    ".github\workflows",
    ".github\ISSUE_TEMPLATE",
    "docs",
    "hardware",
    "src\rs50_thermal"
)

foreach ($folder in $folders) {
    New-Item -ItemType Directory -Force -Path $folder | Out-Null
    Write-Host "   ✅ $folder" -ForegroundColor Green
}

# ─── PASSO 3: Reorganizar arquivos (se estiverem soltos na raiz) ─
Write-Host ""
Write-Host "🧹 Reorganizando arquivos..." -ForegroundColor Yellow

$moves = @(
    @{ From = "BOM.csv";              To = "hardware\bom.csv" }
    @{ From = "config.h";             To = "src\rs50_thermal\config.h" }
    @{ From = "rs50_thermal.ino";     To = "src\rs50_thermal\rs50_thermal.ino" }
    @{ From = "wiring-schematic.svg"; To = "docs\wiring-schematic.svg" }
    @{ From = "calibration-guide.md"; To = "docs\calibration-guide.md" }
    @{ From = "wiring-ascii.txt";     To = "docs\wiring-ascii.txt" }
)

foreach ($move in $moves) {
    if (Test-Path $move.From) {
        Move-Item -Path $move.From -Destination $move.To -Force
        Write-Host "   📄 $($move.From) → $($move.To)" -ForegroundColor Cyan
    }
}

# Move ISSUE_TEMPLATE da raiz se existir como pasta
if ((Test-Path "ISSUE_TEMPLATE") -and -not (Test-Path "ISSUE_TEMPLATE" -PathType Container -ErrorAction SilentlyContinue)) {
    # Skip se já está em .github
} elseif (Test-Path "ISSUE_TEMPLATE\*") {
    Move-Item -Path "ISSUE_TEMPLATE\*" -Destination ".github\ISSUE_TEMPLATE\" -Force
    Remove-Item -Path "ISSUE_TEMPLATE" -Force -ErrorAction SilentlyContinue
    Write-Host "   📁 ISSUE_TEMPLATE → .github\ISSUE_TEMPLATE" -ForegroundColor Cyan
}

# Move workflows da raiz se existir como pasta
if (Test-Path "workflows\*") {
    Move-Item -Path "workflows\*" -Destination ".github\workflows\" -Force
    Remove-Item -Path "workflows" -Force -ErrorAction SilentlyContinue
    Write-Host "   📁 workflows → .github\workflows" -ForegroundColor Cyan
}

# Remove arquivos lixo
$trash = @("FastLED.txt", "info.txt", "arduino build.txt", "arduino-build.txt")
foreach ($file in $trash) {
    if (Test-Path $file) {
        Remove-Item -Path $file -Force
        Write-Host "   🗑️  Removido: $file" -ForegroundColor Gray
    }
}

# ─── PASSO 4: Verificar arquivos críticos ───────────────────────
Write-Host ""
Write-Host "🔍 Verificando arquivos obrigatórios..." -ForegroundColor Yellow

$requiredFiles = @(
    "README.md",
    "LICENSE",
    "CHANGELOG.md",
    ".gitignore",
    "src\rs50_thermal\rs50_thermal.ino",
    "src\rs50_thermal\config.h",
    "docs\calibration-guide.md",
    "docs\wiring-schematic.svg",
    "hardware\bom.csv",
    ".github\workflows\arduino-build.yml",
    ".github\workflows\release.yml",
    ".github\ISSUE_TEMPLATE\bug_report.md",
    ".github\ISSUE_TEMPLATE\feature_request.md"
)

$missing = @()
foreach ($file in $requiredFiles) {
    if (Test-Path $file) {
        Write-Host "   ✅ $file" -ForegroundColor Green
    } else {
        Write-Host "   ❌ FALTANDO: $file" -ForegroundColor Red
        $missing += $file
    }
}

if ($missing.Count -gt 0) {
    Write-Host ""
    Write-Host "⚠️  Arquivos faltando! Crie-os antes de continuar." -ForegroundColor Red
    Write-Host "    Arquivos pendentes:" -ForegroundColor Yellow
    $missing | ForEach-Object { Write-Host "      - $_" -ForegroundColor Yellow }
    Write-Host ""
    $continue = Read-Host "Deseja continuar mesmo assim? (s/N)"
    if ($continue -ne "s") { exit 1 }
}

# ─── PASSO 5: Verificar credenciais WiFi expostas ───────────────
Write-Host ""
Write-Host "🔐 Verificando credenciais WiFi em config.h..." -ForegroundColor Yellow

$configPath = "src\rs50_thermal\config.h"
if (Test-Path $configPath) {
    $configContent = Get-Content $configPath -Raw
    if ($configContent -match 'WIFI_SSID\s+"([^"]+)"') {
        $ssid = $matches[1]
        if ($ssid -ne "SuaRedeAqui" -and $ssid -ne "YourWiFiHere") {
            Write-Host "   ⚠️  WIFI_SSID parece conter dados reais: '$ssid'" -ForegroundColor Red
            Write-Host "   ⚠️  Edite config.h e troque para 'SuaRedeAqui' antes do commit!" -ForegroundColor Red
            $continue = Read-Host "   Continuar mesmo assim? (s/N)"
            if ($continue -ne "s") { exit 1 }
        } else {
            Write-Host "   ✅ Placeholders WiFi OK" -ForegroundColor Green
        }
    }
}

# ─── PASSO 6: Estrutura final ───────────────────────────────────
Write-Host ""
Write-Host "📂 Estrutura final do projeto:" -ForegroundColor Yellow
Get-ChildItem -Recurse -Force -Path . | 
    Where-Object { $_.FullName -notmatch '\\\.git\\' -and $_.Name -ne ".git" } |
    ForEach-Object {
        $depth = ($_.FullName.Replace($PWD.Path, "").Split('\').Count - 2)
        $indent = "  " * $depth
        $icon = if ($_.PSIsContainer) { "📁" } else { "📄" }
        Write-Host "$indent$icon $($_.Name)" -ForegroundColor Cyan
    }

# ─── PASSO 7: Inicializar Git (se necessário) ───────────────────
Write-Host ""
Write-Host "🔧 Configurando Git..." -ForegroundColor Yellow

if (-not (Test-Path ".git")) {
    git init | Out-Null
    Write-Host "   ✅ Repositório Git inicializado" -ForegroundColor Green
} else {
    Write-Host "   ℹ️  Repositório Git já existe" -ForegroundColor Gray
}

# Configurar remote
$remoteExists = git remote 2>&1 | Select-String "origin"
if ($remoteExists) {
    git remote set-url origin $REMOTE_URL
    Write-Host "   ✅ Remote 'origin' atualizado: $REMOTE_URL" -ForegroundColor Green
} else {
    git remote add origin $REMOTE_URL
    Write-Host "   ✅ Remote 'origin' adicionado: $REMOTE_URL" -ForegroundColor Green
}

# ─── PASSO 8: Confirmar antes do commit ─────────────────────────
Write-Host ""
Write-Host "════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  PRONTO PARA COMMITAR" -ForegroundColor Cyan
Write-Host "════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""
Write-Host "Próximas ações:" -ForegroundColor Yellow
Write-Host "  1. git add ." -ForegroundColor White
Write-Host "  2. git commit (mensagem inicial v3.3.0)" -ForegroundColor White
Write-Host "  3. git branch -M main" -ForegroundColor White
Write-Host "  4. git push -u origin main" -ForegroundColor White
Write-Host "  5. git tag v3.3.0 + push" -ForegroundColor White
Write-Host ""
$confirm = Read-Host "Continuar com commit e push? (s/N)"
if ($confirm -ne "s") {
    Write-Host "❌ Cancelado pelo usuário" -ForegroundColor Yellow
    exit 0
}

# ─── PASSO 9: Add, Commit, Push ─────────────────────────────────
Write-Host ""
Write-Host "📝 Preparando commit..." -ForegroundColor Yellow

git add .
git status --short

Write-Host ""
Write-Host "💾 Criando commit..." -ForegroundColor Yellow

$commitMessage = @"
feat: initial release v3.3.0 - thermal controller for RS50 DD wheel

- ESP32-S3-Zero thermal controller
- NTC 100K B3950 temperature monitoring
- Adaptive stator temperature estimation
- Fail-safe relay protection (Songle SLA-24VDC-SL-C)
- Configurable WS2812 LEDs (1/3/5+ modes)
- WiFi WebSocket dashboard
- Persistent hour counter (NVS)
- GitHub Actions CI/CD
"@

git commit -m $commitMessage

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Erro no commit. Verifique se há alterações pendentes." -ForegroundColor Red
    exit 1
}

Write-Host "✅ Commit criado" -ForegroundColor Green

# Renomear branch para main
git branch -M main

# Push
Write-Host ""
Write-Host "🚀 Enviando para GitHub..." -ForegroundColor Yellow
git push -u origin main

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "⚠️  Push falhou. Possíveis causas:" -ForegroundColor Yellow
    Write-Host "   - Repositório no GitHub já tem commits (README inicial)" -ForegroundColor Gray
    Write-Host "     Solução: git pull origin main --allow-unrelated-histories" -ForegroundColor White
    Write-Host "   - Autenticação falhou" -ForegroundColor Gray
    Write-Host "     Solução: configure o Git Credential Manager" -ForegroundColor White
    exit 1
}

Write-Host "✅ Branch main enviada com sucesso!" -ForegroundColor Green

# ─── PASSO 10: Tag de Release ───────────────────────────────────
Write-Host ""
Write-Host "🏷️  Criando tag $VERSION..." -ForegroundColor Yellow

$tagMessage = @"
Release $VERSION - First public release

Features:
- Thermal protection with stator temperature estimation
- Configurable LED visualization (1, 3, 5+ WS2812)
- WiFi dashboard with WebSocket
- Fail-safe relay protection
- Persistent hour counter

Hardware: ESP32-S3-Zero + Songle SLA-24VDC-SL-C + NTC 100K
Target: RS50 Clone DD wheel + Hoverboard 15Nm + ODESC FFBeast
"@

git tag -a $VERSION -m $tagMessage
git push origin $VERSION

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Tag $VERSION enviada!" -ForegroundColor Green
} else {
    Write-Host "⚠️  Falha ao enviar a tag" -ForegroundColor Yellow
}

# ─── FIM ────────────────────────────────────────────────────────
Write-Host ""
Write-Host "╔══════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║              ✅ TUDO PRONTO!                      ║" -ForegroundColor Green
Write-Host "╚══════════════════════════════════════════════════╝" -ForegroundColor Green
Write-Host ""
Write-Host "🔗 Acesse:" -ForegroundColor Cyan
Write-Host "   Repo:     https://github.com/$GITHUB_USER/$REPO_NAME" -ForegroundColor White
Write-Host "   Actions:  https://github.com/$GITHUB_USER/$REPO_NAME/actions" -ForegroundColor White
Write-Host "   Releases: https://github.com/$GITHUB_USER/$REPO_NAME/releases" -ForegroundColor White
Write-Host ""
Write-Host "🎉 Os GitHub Actions devem rodar automaticamente!" -ForegroundColor Yellow
Write-Host "   Aguarde 3-5 minutos para a release com firmware compilado." -ForegroundColor Gray
Write-Host ""
