#!/usr/bin/env bash

rm -f h2.efuse
espefuse.py --chip esp32h2 --virt --path-efuse-file h2.efuse burn_efuse BLK_VERSION_MAJOR 1
