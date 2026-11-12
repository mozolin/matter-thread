@echo off
espefuse --chip esp32h2 --virt --path-efuse-file efuse/h2.efuse burn_efuse BLK_VERSION_MAJOR 1
