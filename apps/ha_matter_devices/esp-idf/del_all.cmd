@echo off
IF EXIST "build" (
  rmdir /s /q build
)
IF EXIST "managed_components" (
  rmdir /s /q managed_components
)
IF EXIST "dependencies.lock" (
  del /f /q dependencies.lock
)
IF EXIST "sdkconfig" (
  del /f /q sdkconfig
)
IF EXIST "sdkconfig.old" (
  del /f /q sdkconfig.old
)
