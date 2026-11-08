#!/usr/bin/env bash

esp-emu \
  --chip esp32c3 \
  --firmware build/merged-binary.bin \
  --elf build/light.elf \
  --ble-hci tcp:localhost:9544 \
  --net "tap,ifname=tap0"
  #--net "user,mdns-nat=yes"
