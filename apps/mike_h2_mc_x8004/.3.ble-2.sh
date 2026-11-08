#!/usr/bin/env bash

##-- Port already in use
#ps aux | grep -E "btvirt|vhci_bridge|esp-emu"
sudo pkill -f btvirt
sudo pkill -f vhci_bridge
sudo pkill -f esp-emu

lsof -i :9545
sudo btvirt -l2 &
sleep 5

hciconfig -a

##-- Start the VHCI bridge
python3 tools/vhci_bridge.py --port 9545 &
#
# [*] Opening /dev/vhci...
# [+] VHCI adapter registered as hci1
#     Use: chip-tool pairing ble-wifi ... --ble-controller 1
# [*] Starting TCP server on port 9544, waiting for esp-emu...
#
