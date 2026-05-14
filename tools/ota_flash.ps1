<#
.SYNOPSIS
  RS50 Thermal Controller — OTA Flash (Windows nativo)
.DESCRIPTION
  Envia firmware compilado para o endpoint /update via HTTP POST.
.PARAMETER Host
  Hostname ou IP do RS50 (default: rs50.local)
.PARAMETER Firmware
  Caminho do .bin (default: src/rs50_thermal/build/rs50_thermal.ino.bin)
.EXAMPLE
  .\tools\ota_flash.ps1
  .\tools\ota_flash.ps1 -RsHost 192.168.1.42
#>
param(
  [string]$RsHost = "rs50.local",
  [string]$Firmware = ""
)
$ErrorActionPreference = "Stop"

$scriptDir   = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir
$sketchDir   = Join-Path $projectRoot "src\rs50_thermal"
$buildDir    = Join-Path $sketchDir   "build"
if (-not $Firmware) { $Firmware = Join-Path $buildDir "rs50_thermal.ino.bin" }

if (-not (Test-Path $Firmware)) {
  Write-Host "❌ Firmware não encontrado: $Firmware" -ForegroundColor Red
  Write-Host "   Compile primeiro com:"
  Write-Host "     arduino-cli compile ``"
  Write-Host "       --fqbn esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashMode=qio,FlashSize=4M,PartitionScheme=default,PSRAM=disabled ``"
  Write-Host "       --output-dir $buildDir $sketchDir"
  exit 1
}

$size = (Get-Item $Firmware).Length
Write-Host "📦 Firmware: $Firmware ($size bytes)"
Write-Host "📡 Destino:  http://$RsHost/update"

if (Test-Connection -ComputerName $RsHost -Count 1 -Quiet -ErrorAction SilentlyContinue) {
  Write-Host "✅ Host alcançável" -ForegroundColor Green
} else {
  Write-Host "⚠️  Host não responde a ping (tentando assim mesmo)" -ForegroundColor Yellow
}

Write-Host "🚀 Iniciando upload..."
try {
  $form = @{ firmware = Get-Item $Firmware }
  $resp = Invoke-WebRequest -Uri "http://$RsHost/update" -Method Post -Form $form -TimeoutSec 120
  if ($resp.StatusCode -eq 200) {
    Write-Host "✅ OTA concluído (HTTP 200)" -ForegroundColor Green
    Write-Host $resp.Content
    Write-Host "🔄 Dispositivo reiniciando..."
  } else {
    Write-Host "❌ OTA retornou HTTP $($resp.StatusCode)" -ForegroundColor Red
    Write-Host $resp.Content
    exit 2
  }
} catch {
  Write-Host "❌ Falha no OTA: $_" -ForegroundColor Red
  exit 2
}
