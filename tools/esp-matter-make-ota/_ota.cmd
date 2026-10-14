@echo off
set PROJECT_NAME=mike_h2_multiclimate
set OTA_SCRIPT_PATH=D:/Espressif/esp-matter/connectedhomeip/connectedhomeip/src/app

if exist ".\build\%PROJECT_NAME%.ota" (
  del /f /q ".\build\%PROJECT_NAME%.ota"
)

python3 %OTA_SCRIPT_PATH%/ota_image_tool.py create -v 0xFFF1 -p 0x8000 -vn 15 -vs "1.0.1" -da sha256 build/%PROJECT_NAME%.bin build/%PROJECT_NAME%.ota

python3 %OTA_SCRIPT_PATH%/ota_image_tool.py show build/%PROJECT_NAME%.ota
