#!/usr/bin/env bash

cp -r $IDF_PATH/examples/openthread/ot_cli /tmp/ot_cli_c6
echo 'CONFIG_OPENTHREAD_NETWORK_AUTO_START=y' > /tmp/ot_cli_c6/sdkconfig.emu
cd /tmp/ot_cli_c6
SDKCONFIG_DEFAULTS='sdkconfig.defaults;sdkconfig.emu' idf.py set-target esp32c6
SDKCONFIG_DEFAULTS='sdkconfig.defaults;sdkconfig.emu' idf.py build
cd build && python -m esptool --chip esp32c6 merge_bin -o merged_flash.bin @flash_args
