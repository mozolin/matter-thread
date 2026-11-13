#!/usr/bin/env bash

esp-emu \
  --chip esp32h2 \
  --firmware build/merged-binary.bin \
  --elf build/mike_h2_multiclimate.elf
  #--efuse ../lib/efuse/h2.efuse
