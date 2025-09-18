@echo off
nrfutil nrf5sdk-tools pkg generate --hw-version 52 --sd-req=0x00  --application build/merged.hex --application-version 1 merged.zip
nrfutil nrf5sdk-tools pkg generate --hw-version 52 --sd-req=0x00  --application build/coprocessor/zephyr/zephyr.hex --application-version 1 zephyr.zip
