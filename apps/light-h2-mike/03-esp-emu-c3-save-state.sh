#!/usr/bin/env bash

# Work on a copy so the original firmware stays clean
cp build/merged-binary.bin saved/merged-binary.bin

# Start bridge + esp-emu with --save-state
#python3 tools/vhci_bridge.py --port 9544 &
esp-emu \
  --chip esp32c3 \
  --firmware saved/merged-binary.bin \
  --elf build/light.elf \
  --ble-hci tcp:localhost:9544 \
  #--net "user,mdns-nat=yes" \
  --save-state

# Commission while running...
# chip-tool pairing ble-wifi 1 MIKE_WIFI_24 66nilozom 20202021 3840 --ble-controller 1

# Stop esp-emu with Ctrl+C — flash is saved on clean exit
