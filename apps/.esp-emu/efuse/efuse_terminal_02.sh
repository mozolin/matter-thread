#!/usr/bin/env bash

espefuse.py --chip esp32h2 --before no_reset -p socket://127.0.0.1:5555 burn_efuse BLK_VERSION_MAJOR 1

#espefuse.py --chip esp32h2 --before no_reset -p socket://127.0.0.1:5555 summary > results/h2.efuse.summary.txt
