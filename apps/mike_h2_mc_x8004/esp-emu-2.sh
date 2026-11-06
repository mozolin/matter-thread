#!/usr/bin/env bash

#rm -rf ~/.esp-emu/

esp-emu --chip esp32h2 \
        --firmware build/merged-binary.bin \
        --elf build/mike_h2_multiclimate.elf \
        --ble-hci tcp:localhost:9544
        #--thread-sim "bind:9002,peer:127.0.0.1:9001"
