# create-schematic.ps1
# Gera o schematic completo do RS50

$ErrorActionPreference = "Stop"

Write-Host "================================================" -ForegroundColor Cyan
Write-Host "  ETAPA 4B: Schematic Generator" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

# Acha o .kicad_sch
$schPath = Get-ChildItem -Recurse -Filter "*.kicad_sch" | Select-Object -First 1
if (-not $schPath) {
    Write-Host "ERRO: nenhum .kicad_sch encontrado!" -ForegroundColor Red
    exit 1
}

# Backup
$backupPath = "$($schPath.FullName).backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
Copy-Item $schPath.FullName $backupPath -Force
Write-Host "Backup salvo: $backupPath" -ForegroundColor Gray
Write-Host ""

# Carrega dados e builder
. "$PSScriptRoot\schematic-data.ps1"
. "$PSScriptRoot\schematic-builder.ps1"

# Monta o schematic
$sb = [System.Text.StringBuilder]::new()
[void]$sb.AppendLine((New-SchematicHeader))
[void]$sb.AppendLine((New-LibSymbolsSection))

Write-Host "Adicionando $($script:Components.Count) componentes..." -ForegroundColor Yellow

# 1. Componentes
foreach ($comp in $script:Components) {
    [void]$sb.AppendLine((New-Component $comp))

    # Adiciona labels nos pinos
    if ($comp.Labels) {
        $i = 0
        foreach ($pin in $comp.Labels.Keys | Sort-Object) {
            $label = $comp.Labels[$pin]
            if ([string]::IsNullOrWhiteSpace($label)) { continue }

            # Posiciona label proximo ao componente, deslocado
            $labelX = $comp.X + 12
            $labelY = $comp.Y + ($i * 2.54) - 5

            if ($label -match "^(\+|GND)") {
                # Power label - ja vai ter power flag separada, so usa label normal
                [void]$sb.AppendLine((New-LocalLabel $label $labelX $labelY))
            } else {
                [void]$sb.AppendLine((New-LocalLabel $label $labelX $labelY))
            }
            $i++
        }
    }
}

# 2. Power flags
Write-Host "Adicionando $($script:PowerFlags.Count) power flags..." -ForegroundColor Yellow
foreach ($pf in $script:PowerFlags) {
    [void]$sb.AppendLine((New-PowerLabel $pf.Net $pf.X $pf.Y))
}

# Footer
[void]$sb.AppendLine((New-SchematicFooter))

# Salva
$content = $sb.ToString()
[System.IO.File]::WriteAllText($schPath.FullName, $content, (New-Object System.Text.UTF8Encoding($false)))

Write-Host ""
Write-Host "================================================" -ForegroundColor Green
Write-Host "  SCHEMATIC GERADO" -ForegroundColor Green
Write-Host "================================================" -ForegroundColor Green
Write-Host ""
Write-Host "Arquivo: $($schPath.FullName)" -ForegroundColor White
Write-Host "Tamanho: $([math]::Round((Get-Item $schPath.FullName).Length / 1KB, 1)) KB" -ForegroundColor White
Write-Host ""
Write-Host "PROXIMOS PASSOS:" -ForegroundColor Cyan
Write-Host "1. Feche o KiCad se estiver aberto" -ForegroundColor White
Write-Host "2. Abra rs50-thermal.kicad_pro" -ForegroundColor White
Write-Host "3. Schematic Editor" -ForegroundColor White
Write-Host "4. Voce vera os componentes posicionados" -ForegroundColor White
Write-Host "5. Use 'w' (wire) pra ligar pinos aos labels mais proximos" -ForegroundColor White
Write-Host ""
Write-Host "Se der erro ao abrir, restaure backup:" -ForegroundColor Yellow
Write-Host "  Copy-Item '$backupPath' '$($schPath.FullName)' -Force" -ForegroundColor Gray
