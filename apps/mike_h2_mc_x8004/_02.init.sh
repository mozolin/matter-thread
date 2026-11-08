#!/usr/bin/env bash

lsof -i :9545
##-- Start the VHCI bridge
python3 tools/vhci_bridge.py --port 9545 &
#
# [*] Opening /dev/vhci...
# [+] VHCI adapter registered as hci1
#     Use: chip-tool pairing ble-wifi ... --ble-controller 1
# [*] Starting TCP server on port 9545, waiting for esp-emu...
#
