# tools/ — Scripts auxiliares do RS50

Scripts utilitários para build, flash e manutenção do firmware.

## `ota_flash.sh` / `ota_flash.ps1`

Envia firmware compilado para o dispositivo via HTTP POST no endpoint `/update`
(ElegantOTA / AsyncElegantOTA).

### Pré-requisitos

1. Dispositivo na mesma rede Wi-Fi do PC
2. mDNS funcionando (`rs50.local`) ou IP fixo conhecido
3. Firmware compilado em `src/rs50_thermal/build/rs50_thermal.ino.bin`

### Build (arduino-cli)

```bash
arduino-cli compile \
  --fqbn esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashMode=qio,FlashSize=4M,PartitionScheme=default,PSRAM=disabled \
  --output-dir src/rs50_thermal/build \
  src/rs50_thermal
```

### Uso

**Linux/macOS/WSL:**

```bash
./tools/ota_flash.sh                          # rs50.local + bin default
./tools/ota_flash.sh 192.168.1.42             # IP custom
./tools/ota_flash.sh rs50.local custom.bin    # bin custom
```

**Windows (PowerShell):**

```powershell
.\tools\ota_flash.ps1                         # rs50.local + bin default
.\tools\ota_flash.ps1 -RsHost 192.168.1.42    # IP custom
.\tools\ota_flash.ps1 -Firmware .\custom.bin  # bin custom
```

### Códigos de saída

| Code | Significado                       |
|------|-----------------------------------|
| 0    | OTA concluído com sucesso         |
| 1    | Firmware não encontrado           |
| 2    | Falha na transferência HTTP       |

### Troubleshooting

- **`rs50.local` não resolve**: instale Bonjour (Windows) ou use IP direto
- **HTTP 401**: ElegantOTA com auth ativa — adicionar `-u user:pass` no curl
- **HTTP 500**: firmware corrompido ou incompatível com bootloader
- **Timeout**: rede Wi-Fi lenta — aumentar `--max-time` (sh) ou `-TimeoutSec` (ps1)
