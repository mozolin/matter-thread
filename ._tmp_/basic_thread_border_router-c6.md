
# Example: OpenThread Border Router + End Device + ChipTool
# Based on the single ESP32-C6 DevBoard!

This example is implemented using the ESP32-C6 development board.  
![](images/ctrl/ESP32-C6-WROOM-1_06.jpg)
Blink GPIO number: 8  
  
Check ESP board configuration:
~~~
esptool -p {COM-PORT} flash_id
~~~
> esptool.py v4.8.1  
> Detecting chip type... ESP32-C6  
> Detected flash size: 8MB  

Build the esp-idf/examples/openthread/ot_rcp example. The firmware doesn't need to be explicitly flashed to a device. It will be included in the Border Router firmware and flashed to the ESP32-H2 chip upon first boot (or the RCP firmware changed).
~~~
cd ~/esp-idf/examples/openthread/ot_rcp
idf.py set-target esp32c6
idf.py build
cd ~/esp-thread-br/examples/basic_thread_border_router
idf.py set-target esp32c6
idf.py menuconfig
~~~
+++ ESP Thread Border Router Example -> Border router board type -> Standalone dev kits
