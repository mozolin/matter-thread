#!/bin/bash

set -e

PROJECT_NAME="mike_h2_multiclimate"
OTA_SCRIPT_PATH="$HOME/esp-matter/connectedhomeip/connectedhomeip/src/app"
OTA_FILE="./build/${PROJECT_NAME}.ota"
BIN_FILE="./build/${PROJECT_NAME}.bin"

# Удаляем старый .ota файл, если он существует
if [ -f "$OTA_FILE" ]; then
    rm -f "$OTA_FILE"
fi

# Проверяем, существует ли .bin файл
if [ ! -f "$BIN_FILE" ]; then
    echo "Ошибка: файл $BIN_FILE не найден. Сначала соберите проект."
    exit 1
fi

python3 "${OTA_SCRIPT_PATH}/ota_image_tool.py" create \
    -v 0xFFF1 \
    -p 0x8000 \
    -vn 15 \
    -vs "1.0.1" \
    -da sha256 \
    "$BIN_FILE" \
    "$OTA_FILE"

echo "OTA-образ успешно создан: $OTA_FILE"

python3 "${OTA_SCRIPT_PATH}/ota_image_tool.py" show "$OTA_FILE"
