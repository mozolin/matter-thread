#!/usr/bin/env bash

sudo hciconfig hci0 down

rm -rf ~/.esp-emu/

esp-emu --chip esp32h2 \
        --firmware build/merged-binary.bin \
        --elf build/mike_h2_multiclimate.elf \
        --net tap,ifname=tap0 \
        --ble-hci hci0
