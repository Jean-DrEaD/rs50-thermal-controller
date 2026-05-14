#!/usr/bin/env bash
# ============================================================================
#  RS50 Thermal — OTA Flash Script
#  Usage: ./tools/flash_ota.sh [host] [firmware.bin]
#  Defaults: host=rs50-thermal.local  bin=build/rs50_thermal.ino.bin
# ============================================================================
set -euo pipefail

HOST="${1:-rs50-thermal.local}"
BIN="${2:-build/rs50_thermal.ino.bin}"
PORT="${OTA_PORT:-3232}"
PASS="${OTA_PASS:-}"

# Localiza espota.py do core arduino-esp32 (Linux/macOS/WSL)
ESPOTA="$(find "${HOME}/.arduino15" "${HOME}/Library/Arduino15" \
              -name espota.py 2>/dev/null | head -n1 || true)"

if [[ -z "${ESPOTA}" ]]; then
  echo "❌ espota.py não encontrado. Instale o core esp32 via arduino-cli."
  exit 1
fi

if [[ ! -f "${BIN}" ]]; then
  echo "❌ Binário não encontrado: ${BIN}"
  echo "   Compile antes com: arduino-cli compile --output-dir build ..."
  exit 1
fi

echo "🔍 Verificando host ${HOST}..."
if ! ping -c1 -W2 "${HOST}" >/dev/null 2>&1; then
  echo "⚠️  ${HOST} não responde a ping (mDNS pode estar ok mesmo assim)"
fi

echo "📤 Enviando ${BIN} → ${HOST}:${PORT}"
python3 "${ESPOTA}" \
  -i "${HOST}" \
  -p "${PORT}" \
  ${PASS:+-a "${PASS}"} \
  -f "${BIN}" \
  -r

echo "✅ OTA concluído."
