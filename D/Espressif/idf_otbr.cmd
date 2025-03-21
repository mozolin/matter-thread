@echo off

set APP_DISK=D:
set APP_PATH=/Espressif/esp-thread-br/examples/basic_thread_border_router
set COM_PORT=COM3


rem  -- Go to app folder and start monitor
cd %APP_PATH%
echo [96mStart monitoring...[0m
idf.py -p %COM_PORT% monitor

@echo.
echo [93mBye, see you later...[0m
@echo.
