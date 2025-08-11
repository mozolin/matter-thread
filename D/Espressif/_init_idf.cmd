@echo off

set NET_DISK=U:
set LINUX_PATH=\\wsl.localhost\Ubuntu-22.04

rem -- Map Ubintu path to local disk
net use %NET_DISK% %LINUX_PATH%
rem -- Change disk letter
%NET_DISK%

exit
