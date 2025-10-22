@echo off

set CHIP_TYPE=esp32s3
set COM_PORT=COM5
set COM_BAUD=460800
set FLASH_SIZE=4MB
rem -- from "sdkconfig": CONFIG_ESPTOOLPY_FLASHFREQ="48m"
set FLASH_FREQ=80m

esptool --chip %CHIP_TYPE% -p %COM_PORT% -b %COM_BAUD% --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq %FLASH_FREQ% --flash_size 4MB 0x0 bootloader/bootloader.bin 0x8000 partition_table/partition-table.bin 0xf000 ota_data_initial.bin 0x20000 esp_ot_br.bin 0x317000 web_storage.bin 0x3ad000 rcp_fw.bin
