
# 3. Thread End Device (ESP32-C6 development board, Ubuntu only):
Source code: [~/esp-matter/examples/light](https://github.com/espressif/esp-matter/tree/main/examples/light)  
This example is implemented using the ESP32-C6 development board.  
![](../images/ctrl/ESP32-C6-WROOM-1_06.jpg)  
  
- Make a copy of /root/esp-matter/examples/light folder to ../light-c6
- Open this folder in VSCode using a remote WSL or in Ubuntu

Navigate to the light example directory, Clean previous build files, Set the build target to ESP32-C6 and Enter the configuration menu
~~~
cd /root/esp-matter/examples/light-c6
rm -rf build/
idf.py set-target esp32c6
idf.py menuconfig
~~~

![](../images/matter/esp_menuconfig_01.png)  
  
![](../images/matter/esp_menuconfig_02.png)  
  
![](../images/matter/esp_menuconfig_03.png)  
  
![](../images/matter/esp_menuconfig_04.png)  
  
![](../images/matter/esp_menuconfig_05.png)  
  
![](../images/matter/esp_menuconfig_06.png)  
  
Building, flashing and monitoring
~~~
idf.py -p /tty/ACM0 build flash monitor
~~~

## Join the Thread Network via NetworkKey or DataSet
**OpenThread Border Router** (see: [How to setup and work with OpenThread Border Router](../OPENTHREAD.md))
~~~
dataset active -x
~~~
> 0e08000000000001000000030000154a0300001735060004001fffe00208def5e21b6165cc560708fde61aeab4004131051000112233445566778899aabbccddeeff030f4f70656e5468726561642d32326339010222c90410a5e0c5822c1e723956af6b1ee43f084e0c0402a0f7f8  
> Done

~~~
networkkey
~~~
> 00112233445566778899aabbccddeeff  
> Done

**Thread End Device**
~~~
matter esp ot_cli factoryreset
matter onboardingcodes none
~~~
> QRCode:            MT:Y.K90-Q000KA0648G00  
> QRCodeUrl:         https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3AY.K90-Q000KA0648G00  
> ManualPairingCode: 34970112332  
~~~
matter config
~~~
> VendorId:        65521 (0xFFF1)  
> ProductId:       32768 (0x8000)  
> HardwareVersion: 1 (0x1)  
> PinCode:         020202021  
> Discriminator:   f00 (3840) 
    
There is a problem joining it to the Thread Network using the border router **dataset**:
~~~
matter esp ot_cli dataset set active 0e08000000000001000000030000154a0300001735060004001fffe00208def5e21b6165cc560708fde61aeab4004131051000112233445566778899aabbccddeeff030f4f70656e5468726561642d32326339010222c90410a5e0c5822c1e723956af6b1ee43f084e0c0402a0f7f8
~~~
> Error 7: InvalidArgs  

Anyway, we can use **networkkey** to join end device to Thread Network:
~~~
matter esp ot_cli dataset networkkey 00112233445566778899aabbccddeeff
matter esp ot_cli dataset commit active
matter esp ot_cli ifconfig up
matter esp ot_cli thread start
~~~

Check End Device state
~~~
matter esp ot_cli state
~~~
Less possible state
> child

Most possible state
> router

![](../images/matter/esp_join_openthread_01.png)  
  
![](../images/matter/esp_join_openthread_02.png)  
  
![](../images/matter/esp_join_openthread_03.png)  
  
![](../images/matter/esp_join_openthread_04.png)  
  
![](../images/matter/esp_join_openthread_05.png)  
  
![](../images/matter/matter_esp_ot_cli.png)  
  
Pressed BOOT button (LED on the board set to OFF):
> I (454109) app_driver: Toggle button pressed  
> I (454109) esp_matter_attribute: R : Endpoint 0x0001's Cluster 0x00000006's Attribute 0x00000000 is 1  
> I (454109) esp_matter_attribute: W : Endpoint 0x0001's Cluster 0x00000006's Attribute 0x00000000 is 0  
> I (454109) led_driver_ws2812: led set r:0, g:0, b:0  
  
Alternatively, we can join the end device by using ChipTool  
~~~
chip-tool pairing code-thread 1234 hex:0e080000000000010000000300001a4a0300001635060004001fffe002083dd5846a27dd139f0708fdec29c2f04b4b23051045005945ef9dbed88082d208673dad0f030f4f70656e5468726561642d3562393101025b9104109855950ef75071da53e996c50694576a0c0402a0f7f8 34970112332
~~~
Command: chip-tool pairing code-thread {node_id} hex:{Dataset} {ManualPairingCode}  
- {node_id} : NodeId chosen for the end device during pairing  
- {Dataset} : from "dataset active -x"  
- {ManualPairingCode} : from "matter onboardingcodes none"  
  
We can toggle LED on board using ChipTool command:  
~~~
chip-tool onoff toggle 1234 1
~~~
NodeId = *1234* (dec) = *0x4d2* (hex)  
> Command: onoff toggle 1 1  
> Sending command to node *0x4d2*  
> Sending cluster (0x00000006) command (0x00000002) on endpoint 1  
> Received Command Response Status for Endpoint=1 Cluster=0x0000_0006 Command=0x0000_0002 Status=0x0  

Set LED level to 10%
~~~
chip-tool levelcontrol move-to-level 10 0 0 0 1234 0x1
~~~

Matter specific command:  
1) Turn on LED  
2) Turn off LED  
~~~
matter esp attribute set 0x1 0x6 0x0 1
matter esp attribute set 0x1 0x6 0x0 0
~~~
