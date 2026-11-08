#!/usr/bin/env bash

cp -r $IDF_PATH/examples/openthread/ot_br /tmp/ot_br_c6
cat > /tmp/ot_br_c6/sdkconfig.emu <<'EOF'
CONFIG_OPENTHREAD_RADIO_NATIVE=y
CONFIG_OPENTHREAD_RADIO_SPINEL_UART=n
CONFIG_ESP_COEX_SW_COEXIST_ENABLE=y
CONFIG_OPENTHREAD_NETWORK_AUTO_START=y
CONFIG_OPENTHREAD_BR_AUTO_START=y
CONFIG_EXAMPLE_WIFI_SSID="MIKE_WIFI_24"
CONFIG_EXAMPLE_WIFI_PASSWORD="66nilozom"
CONFIG_EXAMPLE_CONNECT_WIFI=y
CONFIG_EXAMPLE_CONNECT_ETHERNET=n
CONFIG_OPENTHREAD_CLI=y
CONFIG_OPENTHREAD_CONSOLE_ENABLE=y
EOF
cd /tmp/ot_br_c6
SDKCONFIG_DEFAULTS='sdkconfig.defaults;sdkconfig.emu' idf.py set-target esp32c6
SDKCONFIG_DEFAULTS='sdkconfig.defaults;sdkconfig.emu' idf.py build
cd build && python -m esptool --chip esp32c6 merge_bin -o merged_flash.bin @flash_args
