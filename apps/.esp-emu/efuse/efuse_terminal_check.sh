#!/usr/bin/env bash

esp-emu --chip esp32h2 --firmware build/merged-binary.bin --efuse h2.efuse --elf build/mike_h2_multiclimate.elf
