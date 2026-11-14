#!/usr/bin/env bash

esp-emu --chip esp32h2 --firmware build/merged-binary.bin --efuse h2.efuse --save-state --strap-mode 0x02 --uart-tcp 127.0.0.1:5555

