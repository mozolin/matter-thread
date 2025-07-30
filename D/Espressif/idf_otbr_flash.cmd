@echo off

set APP_DISK=D:
set APP_PATH=/Espressif/esp-thread-br/examples/basic_thread_border_router
set COM_PORT=COM3

set APP_FULL_PATH=%APP_DISK%%APP_PATH%


C:\Windows\system32\cmd.exe /C "cd /D D:\Espressif\esp-thread-br\examples\basic_thread_border_router/build/bootloader/bootloader.bin"

echo Generated D:/Espressif/esp-thread-br/examples/basic_thread_border_router/build/bootloader/bootloader.bin
C:\Windows\system32\cmd.exe /C "cd /D %APP_FULL_PATH%/build/bootloader/bootloader.bin"
| Bootloader binary size 0x53b0 bytes. 0x2c50 bytes (35%) free.
echo Generated D:/Espressif/esp-thread-br/examples/basic_thread_border_router/build/esp_ot_br.bin
> C:\Windows\system32\cmd.exe /C "cd /D D:\Espre...r/examples/basic_thread_border_router/build/esp_ot_br.bin"
| esp_ot_br.bin binary size 0x168960 bytes. Smallest app partition is 0x190000 bytes. 0x276a0 bytes (10%) free.
> C:\Windows\system32\cmd.exe /C "cd /D D:\Espre...essif/esp-idf/components/esptool_py/run_serial_tool.cmake"
> esptool.py --chip esp32s3 -p COM3 -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 bootloader/bootloader.bin 0x20000 esp_ot_br.bin 0x8000 partition_table/partition-table.bin 0xf000 ota_data_initial.bin 0x359000 rcp_fw.bin 0x340000 web_storage.bin
