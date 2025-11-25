#!/usr/bin/env bash

PORT="/dev/ttyACM2"

python -m esptool --chip esp32h2 -b 460800 --before default_reset --after hard_reset -p $PORT write_flash --flash_mode dio --flash_size 4MB --flash_freq 48m 0x0 build/bootloader/bootloader.bin 0xc000 build/partition_table/partition-table.bin 0x1d000 build/ota_data_initial.bin 0x20000 build/mike_on_off.bin
