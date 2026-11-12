@echo off

D:\Espressif\esp-idf\export.bat

esp-emu --chip esp32h2 --firmware build/merged-binary.bin --efuse efuse/h2.efuse
