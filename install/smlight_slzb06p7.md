# Flash SmLight SLZB-06P7
If there is no prompt (>) when connecting to the COM port via PuTTY or screen and the version command does not respond, this means that the RCP firmware on the CC2652P7 chip is not active or was not written correctly. This is key information!
![](../images/slzb-06p7/slzb-06p7_putty.png)  

## 1) Reset to USB boot mode
Disconnect the USB cable from SmLight SLZB-06P7, press and hold the Reset button, connect the USB cable. Wait about 4 seconds, release and press the Reset button once again. Reset to USB boot mode is done if in addition to the yellow LED, the blue LED is constantly lit. It is possible that you won't be able to enter this mode the first time - try again and again...

## 2) Flash using SmartRF Flash Programmer 2 from TI
- select the port and specify the chip type
![](../images/slzb-06p7/ti_flash_programmer_2_01.png)  

- select the firmware file
![](../images/slzb-06p7/ti_flash_programmer_2_02.png)  

- flash
![](../images/slzb-06p7/ti_flash_programmer_2_03.png)  

- disconnect
![](../images/slzb-06p7/ti_flash_programmer_2_04.png)  

## 3) Connect to the internal Wi-Fi network
![](../images/slzb-06p7/slzb-06p7_wifi_spot.png)  

## 4) Current mode after flashing - USB connection
![](../images/slzb-06p7/slzb-06p7_usb_connection.png)  

## 5) Change mode to Wi-Fi connection
![](../images/slzb-06p7/slzb-06p7_wifi_connection.png)  
