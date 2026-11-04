#!/usr/bin/env bash

idf.py merge-bin
#idf.py build
#esptool.py --chip esp32h2 merge_bin -o build/merged-binary.bin @build/flash_args
