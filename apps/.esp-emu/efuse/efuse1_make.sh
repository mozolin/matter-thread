#!/usr/bin/env bash

rm -f results/h2.efuse
python3 results/make-efuse.py --chip esp32h2 --chip-rev 0.2 -o h2.efuse 

#espefuse.py --chip esp32h2 --virt --path-efuse-file results/h2.efuse summary > results/h2.efuse.summary.txt
