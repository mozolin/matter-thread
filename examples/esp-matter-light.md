
# Make an example project (all the settings are made for ESP32-C6 development board, Ubuntu only)
- Make a copy of /root/esp-matter/examples/light folder to ../light-c6
- Open this folder in VSCode using a remote WSL or in Ubuntu
~~~
cd /root/esp-matter/examples/light-c6    #-- Navigate to the light example directory
rm -rf build/                            #-- Clean previous build files
idf.py set-target esp32c6                #-- Set the build target to ESP32-C6
idf.py menuconfig                        #-- Enter the configuration menu
~~~

CONFIG_OPENTHREAD_ENABLED=y  
CONFIG_ENABLE_WIFI_STATION=n  
CONFIG_USE_MINIMAL_MDNS=n  

 
![](../images/matter/esp_menuconfig_01.png)

 
![](../images/matter/esp_menuconfig_02.png)

 
![](../images/matter/esp_menuconfig_03.png)

 
![](../images/matter/esp_menuconfig_04.png)

 
![](../images/matter/esp_menuconfig_05.png)

 
![](../images/matter/esp_menuconfig_06.png)

~~~
idf.py -p /tty/ACM0 build flash monitor  #-- Building, flashing and monitoring
~~~

**JOIN THE THREAD NETWORK VIA NETWORKKEY**  
OpenThread Border Router (see: [How to setup and work with OpenThread Border Router](../OPENTHREAD.md))
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

Thread End Device:
If the end device role is *detached*,
> OPENTHREAD:[N] Mle-----------: Role disabled -> detached

there will be a problem joining it to the Thread Network using the border router dataset:
~~~
dataset set active 0e08000000000001000000030000154a0300001735060004001fffe00208def5e21b6165cc560708fde61aeab4004131051000112233445566778899aabbccddeeff030f4f70656e5468726561642d32326339010222c90410a5e0c5822c1e723956af6b1ee43f084e0c0402a0f7f8
~~~
> Error 7: InvalidArgs  

When the role becomes *leader*, we can use the border router dataset:
> OPENTHREAD:[N] Mle-----------: Role detached -> leader
~~~
dataset set active 0e08000000000001000000030000154a0300001735060004001fffe00208def5e21b6165cc560708fde61aeab4004131051000112233445566778899aabbccddeeff030f4f70656e5468726561642d32326339010222c90410a5e0c5822c1e723956af6b1ee43f084e0c0402a0f7f8
~~~
> Done  

Anyway, we can use **networkkey** to join end device to Thread Network:
~~~
matter esp ot_cli dataset networkkey 00112233445566778899aabbccddeeff
matter esp ot_cli dataset commit active
matter esp ot_cli ifconfig up
matter esp ot_cli thread start
matter esp ot_cli state
~~~

 
![](../images/matter/esp_join_openthread_01.png)

 
![](../images/matter/esp_join_openthread_02.png)

 
![](../images/matter/esp_join_openthread_03.png)

 
![](../images/matter/esp_join_openthread_04.png)

 
![](../images/matter/esp_join_openthread_05.png)

 
![](../images/matter/matter_esp_ot_cli.png)
