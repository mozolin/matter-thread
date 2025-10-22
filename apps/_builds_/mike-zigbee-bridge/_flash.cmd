@echo off

set CHIP_TYPE=esp32s3
set COM_PORT=COM5
set COM_BAUD=460800
set FLASH_SIZE=4MB
rem -- from "sdkconfig": CONFIG_ESPTOOLPY_FLASHFREQ="80m"
set FLASH_FREQ=80m

esptool -p %COM_PORT% erase-flash

esptool --chip %CHIP_TYPE% -p %COM_PORT% -b %COM_BAUD% --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq %FLASH_FREQ% --flash_size %FLASH_SIZE% 0x0 bootloader/bootloader.bin 0x8000 partition_table/partition-table.bin 0x1f000 ota_data_initial.bin 0x30000 zigbee_bridge.bin
