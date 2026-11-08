#!/usr/bin/env bash

esp-emu \
  --chip esp32h2 \
  --firmware saved/merged-binary.bin \
  --elf build/mike_h2_multiclimate.elf \
  --thread-sim "bind:9002,peer:127.0.0.1:9001"
