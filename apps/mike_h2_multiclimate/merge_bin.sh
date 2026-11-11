#!/usr/bin/env bash

#python3 make-efuse.py --chip esp32h2 --chip-rev 0.0 -o h2.efuse
#espefuse.py --chip esp32h2 --virt --path-efuse-file h2.efuse burn_efuse BLK_VERSION_MAJOR 1
#idf.py qemu efuse-summary

idf.py merge-bin
#idf.py build
#esptool.py --chip esp32h2 merge_bin -o build/merged-binary.bin @build/flash_args
