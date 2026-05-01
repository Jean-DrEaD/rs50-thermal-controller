# ═══════════════════════════════════════════════════════════════════════════
#  RS50 README Update v3.3.2 — Line-based editor (no fragile regex)
# ═══════════════════════════════════════════════════════════════════════════

$ErrorActionPreference = "Stop"

# ─── Git PATH fix ─────────────────────────────────────────────────────────
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    $gitCmd = "C:\Program Files\Git\cmd"
    if (Test-Path "$gitCmd\git.exe") {
        $env:Path += ";$gitCmd"
        Write-Host "[OK] Git PATH fixed for this session" -ForegroundColor Green
    } else {
        Write-Host "[X] Git not found!" -ForegroundColor Red; exit 1
    }
}

function Step  { param($N,$T,$M) Write-Host "[$N/$T] $M" -ForegroundColor Cyan }
function OK    { param($M) Write-Host "  [OK] $M" -ForegroundColor Green }
function Warn  { param($M) Write-Host "  [!]  $M" -ForegroundColor Yellow }
function Fail  { param($M) Write-Host "  [X]  $M" -ForegroundColor Red }

Write-Host ""
Write-Host "================================================================" -ForegroundColor Magenta
Write-Host "    RS50 README v3.3.2 - Roadmap removal + Compile section" -ForegroundColor Magenta
Write-Host "================================================================" -ForegroundColor Magenta
Write-Host ""

if (-not (Test-Path "README.md")) { Fail "README.md nao encontrado!"; exit 1 }

# ─── [1/6] Backup ─────────────────────────────────────────────────────────
Step 1 6 "Backup..."
$BackupDir = ".backup\$(Get-Date -Format 'yyyyMMdd-HHmmss')-readme-v3.3.2"
New-Item -ItemType Directory -Path $BackupDir -Force | Out-Null
Copy-Item "README.md" "$BackupDir\README.md"
OK "Backup -> $BackupDir"

# ─── [2/6] Carregar como array de linhas ──────────────────────────────────
Step 2 6 "Carregando README como array..."
$lines = [System.Collections.Generic.List[string]]::new()
Get-Content "README.md" -Encoding UTF8 | ForEach-Object { $lines.Add($_) }
OK "Total: $($lines.Count) linhas"

# ─── [3/6] Remover secao Roadmap ──────────────────────────────────────────
Step 3 6 "Procurando secao Roadmap..."

$roadmapStart = -1
$roadmapEnd   = -1

for ($i = 0; $i -lt $lines.Count; $i++) {
    # Detecta cabecalho do roadmap (qualquer linha "## ... Roadmap")
    if ($lines[$i] -match '^##\s.*Roadmap') {
        $roadmapStart = $i
        # Procura proxima secao "## " ou final do arquivo
        for ($j = $i + 1; $j -lt $lines.Count; $j++) {
            if ($lines[$j] -match '^##\s') {
                $roadmapEnd = $j - 1
                break
            }
        }
        if ($roadmapEnd -eq -1) { $roadmapEnd = $lines.Count - 1 }
        break
    }
}

if ($roadmapStart -ge 0) {
    # Recua o inicio se a linha anterior for "---" (separador da secao anterior)
    $removeFrom = $roadmapStart
    if ($roadmapStart -gt 0 -and $lines[$roadmapStart - 1].Trim() -eq '---') {
        $removeFrom = $roadmapStart - 1
        # Tambem remove linha em branco antes do "---"
        if ($removeFrom -gt 0 -and $lines[$removeFrom - 1].Trim() -eq '') {
            $removeFrom = $removeFrom - 1
        }
    }
    $count = $roadmapEnd - $removeFrom + 1
    OK "Roadmap encontrado: linhas $($removeFrom+1) a $($roadmapEnd+1) ($count linhas)"
    $lines.RemoveRange($removeFrom, $count)
    OK "Roadmap removido"
} else {
    Warn "Roadmap nao encontrado (pode ja ter sido removido)"
}

# ─── [4/6] Corrigir Fan 80mm -> 120mm na BOM ──────────────────────────────
Step 4 6 "Corrigindo fan 80mm -> 120mm..."
$fixedFan = $false
for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i] -match '\*\*Fan\*\*' -and $lines[$i] -match '80mm') {
        $lines[$i] = $lines[$i] -replace '80mm', '120mm'
        $fixedFan = $true
        OK "Linha $($i+1): fan corrigido para 120mm"
        break
    }
}
if (-not $fixedFan) { Warn "Fan ja estava 120mm ou linha nao encontrada" }

# ─── [5/6] Inserir secao de Compilacao antes de Contributing ──────────────
Step 5 6 "Inserindo secao Compilacao & Build Status..."

$compileSection = @(
    ''
    '## 🛠️ Compilação & Build Status'
    ''
    '> ✅ **Status atual:** O sketch `src/rs50_thermal/rs50_thermal.ino` **compila sem erros nem warnings** na Arduino IDE 2.x com o core Arduino-ESP32 3.0+.'
    ''
    '### Configuração da placa (Arduino IDE)'
    ''
    '| Setting | Valor |'
    '|---------|-------|'
    '| **Board** | ESP32S3 Dev Module |'
    '| **USB CDC On Boot** | Enabled |'
    '| **USB Mode** | Hardware CDC and JTAG |'
    '| **Flash Size** | 4MB (32Mb) |'
    '| **Partition Scheme** | Default 4MB with spiffs |'
    '| **PSRAM** | Disabled (ESP32-S3-Zero) |'
    '| **Upload Speed** | 921600 |'
    ''
    '### Bibliotecas necessárias'
    ''
    '```text'
    'FastLED            >= 3.6.0'
    'WebSockets         >= 2.4.1   (Markus Sattler)'
    'ArduinoJson        >= 7.0.0'
    '```'
    ''
    '### Compilação rápida'
    ''
    '```bash'
    '# Clone'
    'git clone https://github.com/Jean-DrEaD/rs50-thermal-controller.git'
    'cd rs50-thermal-controller/src/rs50_thermal'
    ''
    '# Edite suas credenciais WiFi'
    'nano config.h'
    ''
    '# Abra rs50_thermal.ino na Arduino IDE -> Verify (Ctrl+R)'
    '# Conecte o ESP32-S3-Zero via USB-C -> Upload (Ctrl+U)'
    '```'
    ''
    '### Troubleshooting de upload'
    ''
    '| Sintoma | Solução |'
    '|---------|---------|'
    '| `Failed to connect to ESP32` | Segure **BOOT**, clique **RESET**, solte **BOOT** |'
    '| `Sketch too big` | Mude partição para `Huge APP (3MB No OTA)` |'
    '| `A fatal error occurred: MD5 mismatch` | Reduza Upload Speed para `460800` |'
    '| LEDs não acendem após boot | Verifique `LED_COUNT` em `config.h` e 5V no WS2812 |'
    ''
    '---'
    ''
)

# Procurar a linha "## 🤝 Contribuindo" para inserir antes
$insertAt = -1
for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i] -match '^##\s.*Contribuindo') {
        $insertAt = $i
        # Recua se houver "---" antes (vamos manter o --- depois da nova secao)
        if ($i -gt 1 -and $lines[$i-1].Trim() -eq '' -and $lines[$i-2].Trim() -eq '---') {
            $insertAt = $i - 2
        }
        break
    }
}

# Verifica se a secao ja existe
$alreadyExists = $false
for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i] -match '^##\s.*Compilação\s*&\s*Build') {
        $alreadyExists = $true; break
    }
}

if ($alreadyExists) {
    Warn "Secao 'Compilação & Build Status' ja existe - pulando insercao"
} elseif ($insertAt -ge 0) {
    OK "Inserindo na linha $($insertAt+1) (antes de Contribuindo)"
    $lines.InsertRange($insertAt, [string[]]$compileSection)
    OK "Secao Compilacao inserida ($($compileSection.Count) linhas)"
} else {
    Warn "Secao Contribuindo nao encontrada - adicionando ao final"
    $lines.AddRange([string[]]$compileSection)
}

# ─── [6/6] Salvar e commitar ──────────────────────────────────────────────
Step 6 6 "Salvando README.md..."
# Set-Content com Encoding UTF8 (sem BOM no PS 7+, com BOM no PS 5.1 - ambos funcionam no GitHub)
$lines | Set-Content -Path "README.md" -Encoding UTF8
OK "README.md salvo ($($lines.Count) linhas)"

Write-Host ""
Write-Host "Diff stat:" -ForegroundColor Yellow
git diff --stat README.md
Write-Host ""

$see = Read-Host "Ver diff completo? (s/N)"
if ($see -match '^[sS]') { git diff README.md }

Write-Host ""
$ok = Read-Host "Confirmar commit + push? (s/N)"
if ($ok -notmatch '^[sS]') {
    Warn "Cancelado. README atualizado localmente. Backup em $BackupDir"
    exit 0
}

git add README.md

$msg = @"
docs(readme): v3.3.2 - remove roadmap, add compile/build status section

- Remove 'Roadmap' section (project is feature-complete for v3.x)
- Add 'Compilação & Build Status' section confirming rs50_thermal.ino
  compiles without errors/warnings on Arduino IDE 2.x with ESP32-S3 core 3.x
- Document full board configuration (USB CDC, Flash, Partition, PSRAM)
- List required libraries with minimum versions
- Add upload troubleshooting table (boot mode, sketch size, MD5 errors)
- Fix BOM: fan 80mm -> 120mm (consistent with wiring-ascii.txt and bom.csv)

No firmware changes.
"@

git commit -m $msg
if ($LASTEXITCODE -ne 0) { Fail "Commit falhou"; exit 1 }
OK "Commit criado"

git push origin main
if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "================================================================" -ForegroundColor Green
    Write-Host "             README v3.3.2 PUBLICADO COM SUCESSO!" -ForegroundColor Green
    Write-Host "================================================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "  https://github.com/Jean-DrEaD/rs50-thermal-controller" -ForegroundColor Cyan
    Write-Host "  Backup: $BackupDir" -ForegroundColor Gray
    Write-Host ""
} else {
    Fail "Push falhou. Cheque a conexao."
    exit 1
}