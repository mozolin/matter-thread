@echo off

rem -- The first argument, not empty one, starts monitor mode without flashing firmware
set MONITOR_ONLY=%1

set APP_DISK=D:
set APP_PATH=/Espressif/esp-idf/examples/openthread/ot_cli-c6

set CHIP_TYPE=esp32c6
set COM_PORT=COM5


%APP_DISK%
rem  -- Go to app folder and start monitor
cd %APP_PATH%
echo [93m%cd%[0m

if %MONITOR_ONLY%. NEQ . goto MONITOR_ONLY

@echo.
set /p RESPONSE=Monitor only? (1/0): 
rem -- Pressed <ENTER>
if %RESPONSE%. == . goto FINISHED
rem -- Go to :MONITOR_ONLY
if %RESPONSE% == 1 goto MONITOR_ONLY


rem  -- Flash firmware
idf.py -p %COM_PORT% flash

:MONITOR_ONLY

echo [96mStart monitoring...[0m
idf.py -p %COM_PORT% monitor

:FINISHED
@echo.
echo [93mBye, see you later...[0m
@echo.
