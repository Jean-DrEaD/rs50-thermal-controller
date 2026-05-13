<#
.SYNOPSIS
    Commit + push + PR automático para o firmware RS50.
.EXAMPLE
    .\commit.ps1 -Message "feat: dashboard WS + UDP SimHub"
#>

param(
    [Parameter(Mandatory = $true)]
    [string]$Message,

    [string]$Branch = "",
    [switch]$NoPR,
    [string]$Base = "main",
    [switch]$SkipHooks
)

$ErrorActionPreference = "Stop"

function Write-Step($t) { Write-Host "`n▶ $t" -ForegroundColor Cyan }
function Write-Ok($t)   { Write-Host "✓ $t"   -ForegroundColor Green }
function Write-Warn($t) { Write-Host "⚠ $t"   -ForegroundColor Yellow }
function Write-Err($t)  { Write-Host "✗ $t"   -ForegroundColor Red }

# 1. Repo check
Write-Step "Verificando repositório Git..."
if (-not (Test-Path ".git")) { throw "Não é um repositório Git" }
Write-Ok "Repositório encontrado."

# 2. Mudanças?
Write-Step "Verificando mudanças..."
$changes = git status --porcelain
if (-not $changes) { Write-Warn "Nada para commitar."; exit 0 }
Write-Host $changes

# 3. Branch
if ($Branch) {
    Write-Step "Alternando para branch '$Branch'..."
    $exists = git branch --list $Branch
    if ($exists) { git checkout $Branch } else { git checkout -b $Branch }
} else {
    $Branch = (git rev-parse --abbrev-ref HEAD).Trim()
}
Write-Ok "Branch ativa: $Branch"

# 4. Add
Write-Step "Adicionando arquivos..."
git add -A
if ($LASTEXITCODE -ne 0) { throw "Falha em git add" }

# 5. Commit (CHECANDO EXIT CODE!)
Write-Step "Commitando: '$Message'"
$commitArgs = @("commit", "-m", $Message)
if ($SkipHooks) { $commitArgs += "--no-verify" }
& git @commitArgs
if ($LASTEXITCODE -ne 0) {
    Write-Err "git commit FALHOU (exit $LASTEXITCODE). Provavelmente o pre-commit hook bloqueou."
    Write-Warn "Use -SkipHooks para ignorar (não recomendado) ou corrija o que o hook reclamou."
    exit 1
}
Write-Ok "Commit criado."

# 6. Push
Write-Step "Push para origin/$Branch..."
git push -u origin $Branch
if ($LASTEXITCODE -ne 0) { Write-Err "Push falhou"; exit 1 }
Write-Ok "Push concluído."

# 7. PR
if ($NoPR)              { Write-Warn "PR pulado (--NoPR)"; exit 0 }
if ($Branch -eq $Base)  { Write-Warn "Branch == base ($Base), sem PR"; exit 0 }

Write-Step "Verificando GitHub CLI (gh)..."
if (-not (Get-Command gh -ErrorAction SilentlyContinue)) {
    Write-Warn "'gh' não instalado. Rode: winget install -e --id GitHub.cli"
    Write-Warn "Depois: gh auth login"
    exit 0
}

Write-Step "Criando PR..."
gh pr create --base $Base --head $Branch --title $Message --body "Automatizado por commit.ps1`n`n$Message"
if ($LASTEXITCODE -eq 0) {
    Write-Ok "PR criado."
    gh pr view --web
} else {
    Write-Err "Falha ao criar PR (talvez já exista)."
    gh pr view --web
}
