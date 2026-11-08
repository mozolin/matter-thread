#!/usr/bin/env bash

echo 'CONFIG_OPENTHREAD_NETWORK_AUTO_START=y' > sdkconfig.thread
SDKCONFIG_DEFAULTS='sdkconfig.defaults;sdkconfig.thread' idf.py build

esp-emu \
  --chip esp32h2 \
  --firmware build/merged-binary.bin \
  --elf build/mike_h2_multiclimate.elf \
  --thread-sim "bind:9001,peer:127.0.0.1:9002"
