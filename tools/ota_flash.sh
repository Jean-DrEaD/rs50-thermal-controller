#!/usr/bin/env bash
# RS50 Thermal Controller — OTA Flash (Linux/macOS/WSL)
# Uso: ./tools/ota_flash.sh [host] [firmware.bin]
# Default: host=rs50.local, firmware=src/rs50_thermal/build/rs50_thermal.ino.bin
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
SKETCH_DIR="${PROJECT_ROOT}/src/rs50_thermal"
BUILD_DIR="${SKETCH_DIR}/build"

HOST="${1:-rs50.local}"
DEFAULT_BIN="${BUILD_DIR}/rs50_thermal.ino.bin"
FIRMWARE="${2:-${DEFAULT_BIN}}"

if [[ ! -f "${FIRMWARE}" ]]; then
  echo "❌ Firmware não encontrado: ${FIRMWARE}" >&2
  echo "   Compile primeiro com:" >&2
  echo "     arduino-cli compile \\" >&2
  echo "       --fqbn esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashMode=qio,FlashSize=4M,PartitionScheme=default,PSRAM=disabled \\" >&2
  echo "       --output-dir ${BUILD_DIR} ${SKETCH_DIR}" >&2
  exit 1
fi

SIZE=$(stat -c%s "${FIRMWARE}" 2>/dev/null || stat -f%z "${FIRMWARE}")
echo "📦 Firmware: ${FIRMWARE} (${SIZE} bytes)"
echo "📡 Destino:  http://${HOST}/update"

if command -v ping >/dev/null 2>&1; then
  ping -c 1 -W 2 "${HOST}" >/dev/null 2>&1 \
    && echo "✅ Host alcançável" \
    || echo "⚠️  Host não responde a ping (pode ser firewall — tentando assim mesmo)"
fi

echo "🚀 Iniciando upload..."
RESP_FILE="$(mktemp)"
HTTP_CODE=$(curl -sS -o "${RESP_FILE}" -w "%{http_code}" \
  -F "firmware=@${FIRMWARE}" \
  --max-time 120 \
  "http://${HOST}/update")

if [[ "${HTTP_CODE}" == "200" ]]; then
  echo "✅ OTA concluído (HTTP 200)"
  cat "${RESP_FILE}"
  echo ""
  echo "🔄 Dispositivo reiniciando..."
  rm -f "${RESP_FILE}"
else
  echo "❌ OTA falhou (HTTP ${HTTP_CODE})" >&2
  cat "${RESP_FILE}" >&2
  rm -f "${RESP_FILE}"
  exit 2
fi