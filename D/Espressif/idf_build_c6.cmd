@echo off

rem -- The first argument, not empty one, starts monitor mode without flashing firmware
set MONITOR_ONLY=%1

set NET_DISK=U:
set LINUX_PATH=\\wsl$\Ubuntu
set IDF_PATH=/root/esp-idf
set APP_PATH=/root/esp-matter/examples/light-c6

set CHIP_TYPE=esp32c6
set COM_PORT=COM5
set COM_BAUD=460800
rem -- from "sdkconfig": CONFIG_ESPTOOLPY_FLASHFREQ="80m"
set FLASH_FREQ=80m


echo [30m
rem -- Map Ubintu path to local disk
net use %NET_DISK% %LINUX_PATH%
rem -- Change disk letter
%NET_DISK%
echo [0m

if %MONITOR_ONLY%. NEQ . goto MONITOR_ONLY

@echo.
set /p RESPONSE=Monitor only? (1/0): 
rem -- Pressed <ENTER>
if %RESPONSE%. == . goto FINISHED
rem -- Go to :MONITOR_ONLY
if %RESPONSE% == 1 goto MONITOR_ONLY


rem -- Get size info of bootloader.bin
python %IDF_PATH%/components/partition_table/check_sizes.py --offset 0xc000 bootloader 0x0 %APP_PATH%/build/bootloader/bootloader.bin

rem -- Get size info of partition-table.bin and light.bin
python %IDF_PATH%/components/partition_table/check_sizes.py --offset 0xc000 partition --type app %APP_PATH%/build/partition_table/partition-table.bin %APP_PATH%/build/light.bin

rem -- Set environment
cmake -D IDF_PATH=%IDF_PATH% -D "SERIAL_TOOL=%PRJ_PATH%\.espressif/python_env/idf5.2_py3.10_env/bin/python;;%IDF_PATH%/components/esptool_py/esptool/esptool.py;--chip;%CHIP_TYPE%" -D "SERIAL_TOOL_ARGS=--before=default_reset;--after=hard_reset;write_flash;@flash_args" -D WORKING_DIRECTORY=%APP_PATH%/build  -D ESPBAUD=%COM_BAUD% -D ESPPORT=%COM_PORT% -P %IDF_PATH%/components/esptool_py/run_serial_tool.cmake

rem  -- Go to app build folder and flash firmware
cd %APP_PATH%/build
echo [96mStart firmware flashing...[0m
esptool --chip %CHIP_TYPE% -p %COM_PORT% -b %COM_BAUD% --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq %FLASH_FREQ% --flash_size 4MB 0x0 bootloader/bootloader.bin 0x20000 light.bin 0xc000 partition_table/partition-table.bin 0x1d000 ota_data_initial.bin

:MONITOR_ONLY

rem  -- Go to app folder and start monitor
cd %APP_PATH%
echo [96mStart monitoring...[0m
idf.py -p %COM_PORT% monitor

:FINISHED
@echo.
echo [93mBye, see you later...[0m
@echo.
