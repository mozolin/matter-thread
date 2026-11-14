#!/usr/bin/env bash

espefuse.py --chip esp32h2 --virt --path-efuse-file h2.efuse summary > results/h2.efuse.summary.txt
