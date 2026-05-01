# ================================================================
#  RS50 Thermal Controller -- Update Repository Script
#  Version: v3.3.1 (polishing)
#  Usage: .\update-repo.ps1
# ================================================================

$ErrorActionPreference = "Stop"

# --- CONFIGURATION ---------------------------------------------
$REPO_PATH    = "K:\rs50-thermal-controller"
$GITHUB_USER  = "Jean-DrEaD"
$REPO_NAME    = "rs50-thermal-controller"

# --- BANNER ----------------------------------------------------
Clear-Host
Write-Host ""
Write-Host "==============================================================" -ForegroundColor Magenta
Write-Host "   RS50 Thermal Controller -- Polish Update                   " -ForegroundColor Magenta
Write-Host "   Adicionando README profissional + Banner SVG               " -ForegroundColor Magenta
Write-Host "==============================================================" -ForegroundColor Magenta
Write-Host ""

# --- CHECK DIRECTORY -------------------------------------------
if (-not (Test-Path $REPO_PATH)) {
    Write-Host "[ERRO] Diretorio nao encontrado: $REPO_PATH" -ForegroundColor Red
    Write-Host "       Ajuste a variavel REPO_PATH no inicio do script" -ForegroundColor Yellow
    exit 1
}

Set-Location $REPO_PATH
Write-Host "[INFO] Diretorio: $REPO_PATH" -ForegroundColor Cyan
Write-Host ""

# --- CHECK GIT STATUS ------------------------------------------
Write-Host "[1/9] Verificando estado do Git..." -ForegroundColor Yellow
$gitStatus = git status --porcelain
if ($gitStatus) {
    Write-Host "[AVISO] Ha mudancas nao commitadas:" -ForegroundColor Yellow
    git status --short
    Write-Host ""
    $continue = Read-Host "Continuar mesmo assim? (s/N)"
    if ($continue -ne "s") { exit 0 }
}

# --- PULL ------------------------------------------------------
Write-Host ""
Write-Host "[2/9] Sincronizando com remote..." -ForegroundColor Yellow
try {
    git pull origin main --rebase
    Write-Host "[OK] Sincronizado" -ForegroundColor Green
} catch {
    Write-Host "[AVISO] Pull falhou (talvez nao haja mudancas remotas)" -ForegroundColor Yellow
}

# --- BACKUP README ---------------------------------------------
Write-Host ""
Write-Host "[3/9] Criando backup do README atual..." -ForegroundColor Yellow
if (Test-Path "README.md") {
    Copy-Item "README.md" "README.md.backup" -Force
    Write-Host "[OK] Backup criado: README.md.backup" -ForegroundColor Gray
}

# --- INSTRUCTIONS ----------------------------------------------
Write-Host ""
Write-Host "==============================================================" -ForegroundColor Cyan
Write-Host "  [4/9] ACAO MANUAL NECESSARIA                                " -ForegroundColor Cyan
Write-Host "==============================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Os seguintes arquivos precisam ser ATUALIZADOS/CRIADOS:" -ForegroundColor Yellow
Write-Host ""
Write-Host "  1. README.md              <-- cole o novo README do chat" -ForegroundColor White
Write-Host "  2. docs\banner.svg        <-- cole o banner SVG" -ForegroundColor White
Write-Host "  3. docs\wiring-ascii.txt  <-- cole o diagrama ASCII" -ForegroundColor White
Write-Host ""
Write-Host "DICA: Use VS Code, Notepad++ ou outro editor que SALVE EM UTF-8" -ForegroundColor Yellow
Write-Host ""

# --- OPEN EDITOR -----------------------------------------------
$openEditor = Read-Host "Abrir VS Code agora? (s/N)"
if ($openEditor -eq "s") {
    if (Get-Command code -ErrorAction SilentlyContinue) {
        code .
        Write-Host "[OK] VS Code aberto" -ForegroundColor Green
    } else {
        Write-Host "[AVISO] VS Code nao encontrado, abrindo Explorer..." -ForegroundColor Yellow
        explorer .
    }
}

Write-Host ""
Read-Host "Pressione ENTER quando terminar de atualizar os arquivos"

# --- VERIFY FILES ----------------------------------------------
Write-Host ""
Write-Host "[5/9] Verificando arquivos..." -ForegroundColor Yellow

$filesToCheck = @(
    @{ Path = "README.md";              MinSize = 5000;  Desc = "README profissional" }
    @{ Path = "docs\banner.svg";        MinSize = 1500;  Desc = "Banner SVG" }
    @{ Path = "docs\wiring-ascii.txt";  MinSize = 2000;  Desc = "Diagrama ASCII" }
)

$allOk = $true
foreach ($file in $filesToCheck) {
    if (Test-Path $file.Path) {
        $size = (Get-Item $file.Path).Length
        $sizeKB = [math]::Round($size/1024, 1)
        if ($size -ge $file.MinSize) {
            $msg = "   [OK] " + $file.Path + " (" + $sizeKB + " KB) -- " + $file.Desc
            Write-Host $msg -ForegroundColor Green
        } else {
            $msg = "   [AVISO] " + $file.Path + " parece pequeno demais (" + $size + " bytes)"
            Write-Host $msg -ForegroundColor Yellow
            $allOk = $false
        }
    } else {
        $msg = "   [FALTA] " + $file.Path + " -- " + $file.Desc
        Write-Host $msg -ForegroundColor Red
        $allOk = $false
    }
}

if (-not $allOk) {
    Write-Host ""
    $continue = Read-Host "Alguns arquivos tem problemas. Continuar mesmo assim? (s/N)"
    if ($continue -ne "s") { exit 1 }
}

# --- CHECK README REFERENCES -----------------------------------
Write-Host ""
Write-Host "[6/9] Validando referencias do README..." -ForegroundColor Yellow

$readmeContent = Get-Content "README.md" -Raw -Encoding UTF8
$brokenRefs = @()

$internalRefs = @(
    "docs/banner.svg",
    "docs/wiring-schematic.svg",
    "docs/wiring-ascii.txt",
    "docs/calibration-guide.md",
    "hardware/bom.csv",
    "CHANGELOG.md",
    "LICENSE"
)

foreach ($ref in $internalRefs) {
    if ($readmeContent -match [regex]::Escape($ref)) {
        $localPath = $ref -replace '/', '\'
        if (Test-Path $localPath) {
            Write-Host "   [OK] $ref" -ForegroundColor Green
        } else {
            Write-Host "   [AVISO] $ref (referenciado mas nao existe)" -ForegroundColor Yellow
            $brokenRefs += $ref
        }
    }
}

if ($brokenRefs.Count -gt 0) {
    Write-Host ""
    Write-Host "[AVISO] Links quebrados detectados -- verifique antes de commitar" -ForegroundColor Yellow
}

# --- GITHUB MANUAL CONFIG --------------------------------------
Write-Host ""
Write-Host "==============================================================" -ForegroundColor Cyan
Write-Host "  [7/9] CONFIGURACOES MANUAIS NO GITHUB (opcional)            " -ForegroundColor Cyan
Write-Host "==============================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Acesse: https://github.com/$GITHUB_USER/$REPO_NAME" -ForegroundColor White
Write-Host ""
Write-Host "1. Clique na engrenagem ao lado de 'About' (canto sup. direito)" -ForegroundColor Gray
Write-Host ""
Write-Host "2. Description:" -ForegroundColor Gray
Write-Host "   Sistema de protecao termica open-source para volantes DD" -ForegroundColor White
Write-Host ""
Write-Host "3. Topics (cole estas tags separadas por espaco):" -ForegroundColor Gray
Write-Host "   esp32 esp32-s3 thermal-management sim-racing direct-drive" -ForegroundColor White
Write-Host "   ddwheel arduino ntc-thermistor ffbeast odesc hoverboard-motor" -ForegroundColor White
Write-Host "   ws2812 pwm-fan websocket-dashboard fail-safe" -ForegroundColor White
Write-Host ""
Write-Host "4. Marque: 'Releases' e 'Packages'" -ForegroundColor Gray
Write-Host ""

Read-Host "Pressione ENTER apos configurar (ou para pular)"

# --- DIFF PREVIEW ----------------------------------------------
Write-Host ""
Write-Host "[8/9] Mudancas detectadas:" -ForegroundColor Yellow
git status --short
Write-Host ""

$showDiff = Read-Host "Ver diff completo do README? (s/N)"
if ($showDiff -eq "s") {
    git diff README.md
    Write-Host ""
    Read-Host "Pressione ENTER para continuar"
}

# --- COMMIT ----------------------------------------------------
Write-Host ""
Write-Host "[9/9] Preparando commit..." -ForegroundColor Yellow

git add .

$commitMsg = "docs: professional README with banner and full documentation`n`n" +
             "- New README.md with badges, ToC and structured sections`n" +
             "- Add docs/banner.svg with custom branding`n" +
             "- Add docs/wiring-ascii.txt as text alternative to SVG`n" +
             "- Improve overall documentation structure`n" +
             "- Add roadmap, contributing guide and acknowledgments`n`n" +
             "This is a documentation polish -- no code changes."

Write-Host ""
Write-Host "Mensagem do commit:" -ForegroundColor Cyan
Write-Host $commitMsg -ForegroundColor Gray
Write-Host ""

$confirm = Read-Host "Confirmar commit? (s/N)"
if ($confirm -ne "s") {
    Write-Host "[CANCELADO] Para descartar mudancas: git restore ." -ForegroundColor Yellow
    exit 0
}

git commit -m $commitMsg
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERRO] Falha no commit" -ForegroundColor Red
    exit 1
}

Write-Host "[OK] Commit criado" -ForegroundColor Green

# --- PUSH ------------------------------------------------------
Write-Host ""
Write-Host "Enviando para GitHub..." -ForegroundColor Yellow

git push origin main

if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERRO] Push falhou" -ForegroundColor Red
    exit 1
}

Write-Host "[OK] Push concluido!" -ForegroundColor Green

# --- REMOVE BACKUP ---------------------------------------------
if (Test-Path "README.md.backup") {
    $rmBackup = Read-Host "Remover README.md.backup? (s/N)"
    if ($rmBackup -eq "s") {
        Remove-Item "README.md.backup"
        Write-Host "[OK] Backup removido" -ForegroundColor Gray
    }
}

# --- DONE ------------------------------------------------------
Write-Host ""
Write-Host "==============================================================" -ForegroundColor Green
Write-Host "             ATUALIZACAO CONCLUIDA COM SUCESSO!               " -ForegroundColor Green
Write-Host "==============================================================" -ForegroundColor Green
Write-Host ""
Write-Host "Acesse para ver o resultado:" -ForegroundColor Cyan
Write-Host "   https://github.com/$GITHUB_USER/$REPO_NAME" -ForegroundColor White
Write-Host ""
Write-Host "Proximos passos sugeridos:" -ForegroundColor Yellow
Write-Host "   1. Configurar Topics no GitHub (tags)" -ForegroundColor Gray
Write-Host "   2. Pin do repo no seu perfil" -ForegroundColor Gray
Write-Host "   3. Compartilhar nas comunidades:" -ForegroundColor Gray
Write-Host "      - Discord FFBeast" -ForegroundColor Gray
Write-Host "      - Reddit r/simracing" -ForegroundColor Gray
Write-Host "      - Forum Brasil Sim Racing" -ForegroundColor Gray
Write-Host ""
Write-Host "Para criar proxima release (v3.4.0):" -ForegroundColor Yellow
Write-Host "   git tag -a v3.4.0 -m 'Release v3.4.0'" -ForegroundColor White
Write-Host "   git push origin v3.4.0" -ForegroundColor White
Write-Host ""

# --- OPEN BROWSER ----------------------------------------------
$openBrowser = Read-Host "Abrir o repositorio no navegador? (s/N)"
if ($openBrowser -eq "s") {
    Start-Process "https://github.com/$GITHUB_USER/$REPO_NAME"
}

Write-Host ""
Write-Host "Tudo pronto, Delano! Boa pista!" -ForegroundColor Magenta
Write-Host ""
