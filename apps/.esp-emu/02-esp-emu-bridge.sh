#!/usr/bin/env bash

#-- Port already in use
pkill -f vhci_bridge

#-- Start the VHCI bridge
python3 tools/vhci_bridge.py
#
# [*] Opening /dev/vhci...
# [+] VHCI adapter registered as hci1
#     Use: chip-tool pairing ble-wifi ... --ble-controller 1
# [*] Starting TCP server on port 9544, waiting for esp-emu...
#
