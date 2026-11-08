#!/usr/bin/env bash

#-- VHCI device permissions
sudo chmod 666 /dev/vhci

#-- For a persistent rule
echo 'KERNEL=="vhci", MODE="0666"' | sudo tee /etc/udev/rules.d/99-vhci.rules
sudo udevadm trigger

#-- Port already in use
pkill -f vhci_bridge

#-- Start the VHCI bridge
python3 tools/vhci_bridge.py &
#
# [*] Opening /dev/vhci...
# [+] VHCI adapter registered as hci1
#     Use: chip-tool pairing ble-wifi ... --ble-controller 1
# [*] Starting TCP server on port 9544, waiting for esp-emu...
#


# BR first (port 9001)
esp-emu --chip esp32c6 \
  --firmware /tmp/ot_br_c6/build/merged_flash.bin \
  --elf /tmp/ot_br_c6/build/esp_ot_br.elf \
  --net "user,hostfwd=tcp::18080-:80"
  --thread-sim "bind:9001,peer:127.0.0.1:9002" &
sleep 2

# Device (port 9002)
esp-emu --chip esp32c6 \
  --firmware /tmp/ot_cli_c6/build/merged_flash.bin \
  --elf /tmp/ot_cli_c6/build/esp_ot_cli.elf \
  --net "user,hostfwd=tcp::18080-:80"
  --thread-sim "bind:9002,peer:127.0.0.1:9001" &
wait
