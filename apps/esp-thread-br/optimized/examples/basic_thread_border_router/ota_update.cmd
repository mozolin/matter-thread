@echo off
curl -X POST -H "Content-Type: application/octet-stream" --data-binary "@build/esp_ot_br.bin" http://192.168.1.250/api/ota-update
