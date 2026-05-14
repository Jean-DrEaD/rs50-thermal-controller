#!/usr/bin/env bash
# RS50 Thermal Controller - OTA Flash (Linux/macOS)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/.pio/build"
DEFAULT_ENV="esp32dev"

ENV="${1:-${DEFAULT_ENV}}"
HOST="${2:-rs50.local}"
FIRMWARE="${BUILD_DIR}/${ENV}/firmware.bin"

if [[ ! -f "${FIRMWARE}" ]]; then
  echo "❌ Firmware não encontrado: ${FIRMWARE}"
  echo "   Rode: pio run -e ${ENV}"
  exit 1
fi

echo "📡 Enviando ${FIRMWARE} para http://${HOST}/update"
curl -sS -F "firmware=@${FIRMWARE}" "http://${HOST}/update" \
  && echo "✅ OTA concluído" \
  || { echo "❌ Falha no OTA"; exit 2; }
