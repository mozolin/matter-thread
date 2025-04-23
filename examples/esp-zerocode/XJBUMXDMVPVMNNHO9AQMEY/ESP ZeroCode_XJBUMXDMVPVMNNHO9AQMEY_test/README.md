# Espressif ZeroCode

This document provides instructions to use the ESP ZeroCode product.

We assume that you now have a device with the flash programmed, either through the ZeroCode web portal, or by programming it yourself. If you are flashing the devices yourself from the downloaded zip, refer the [Firmware Flashing](#firmware-flashing) section first. Once done, please resume reading from this section.

> Note: This firmware has test certificates, which cannot be used for a production device. When you place an order, the ZeroCode modules will have the production certificates, as they should.

Please follow these instructions to get your device up and running.

## Setup Code

Scan the below QR Code from any of the Matter supported phone apps (details below) to setup the device.

> Note: Use the QR Code in the device's respective folder if you are flashing multiple devices yourself.

<img src="https://esp-ezc-launchpad-246098634217-prod.s3.us-east-1.amazonaws.com/companies/YJDLIBRGJCKIYBP322ZOR7/products/XJBUMXDMVPVMNNHO9AQMEY/682437025_1734098384/1/qr_code.png" alt="qr_code" width="30%" />

## Device Setup

Here's how you can get started with using this device with any of your favourite Ecosystems:

<details>
<summary>Amazon</summary>

* Requirements:
    * Latest Amazon Alexa app on an Android Device.
    * One of these <a href="https://www.amazon.com/b?ie=UTF8&node=37490568011#:~:text=Echo%20Dot%20(5th%20Gen)%2C,(v3)%2C%20Echo%20Dot%20Gen" target="_blank">devices</a> already setup in your network:
        * Echo (at least 4th gen)
        * Echo Dot (at least 3rd gen)
        * Echo Show (at least 5)
        * Echo Flex
        * Echo Input
        * Echo Plus
        * Echo Pop
        * Echo Studio
* Setup:
    * In the Alexa app, go to devices.
    * Click the `+` icon on the top right, and select `Add Device`.
    * Then scroll to the bottom and select `Other`. Then select `Matter`.
    * You will be asked if your device has a Matter logo. Select `Yes`.
    * Scan the QR code or enter the setup code manually.
    * The device setup should start.
    * You will be prompted to enter your network credentials.
    * In about a minute, you will be prompted to add your device to any room/group and change the device name to what you want.
    * The app should show that the device has been added successfully.
* Control:
    * You should be able to control the device using the Alexa app.
    * You can also use `Alexa` and use voice to control your device.

</details>

<details>
<summary>Apple</summary>

* Requirements:
    * Latest Apple Home app on an Apple device (running at least iOS/iPadOS 16.1).
    * One of these <a href="https://github.com/espressif/connectedhomeip/blob/master/docs/guides/darwin.md#setup-requirements-for-application-development" target="_blank">devices</a> already setup in your network:
        * Apple Homepod (running at least tvOS 16.1)
        * Apple TV (running at least tvOS 16.1)
* Setup:
    * Open the Apple Home app.
    * Click the `+` icon on the top right, and select `Add Accessory`.
    * Scan the QR code or enter the setup code manually.
    * The device setup should start.
    * You might get a prompt that you are adding an `uncertified accessory`, click on `add anyway`.
    * In about a minute, you will be prompted to add your device to any room and change the device name to what you want.
    * You might be asked for some additional device setup, which you can `skip` or do them `later`.
    * The app should show that the device has been added successfully.
* Control:
    * You should be able to control the device using the Apple Home app.
    * You can also use `Siri` and use voice to control your device.

</details>

<details>
<summary>Google</summary>

* Requirements:
    * Latest Google Home app on an Android Device (running at least Android 8.1).
    * One of these <a href="https://support.google.com/googlenest/answer/12391458" target="_blank">devices</a> already setup in your network:
        * Google Home (does not have Thread support)
        * Google Home Mini (does not have Thread support)
        * Google Nest Audio (does not have Thread support)
        * Google Nest Hub (does not have Thread support)
        * Google Nest Hub (2nd gen)
        * Google Nest Hub Max
        * Google Nest Mini (does not have Thread support)
        * Nest WiFi Pro
    * Since the device has test certificates, your Google account needs to be registered to be able to setup the device. You can share your Google email ID with us, and we can enable your account.
* Setup:
    * In the Google Home app, go to devices.
    * Click the `+ Add` button, and select `New device`.
    * Select your home then the app will start looking for nearby devices.
    * Select your device from the list, then scan the QR code or enter the setup code manually.
    * The device setup should start.
    * In about a minute, the app should show that the device has been added successfully.
    * You can now add your device to any room and change the device name to what you want.
* Control:
    * You should be able to control the device using the Google Home app.
    * You can also use `Google Assistant` and use voice to control your device.

</details>

<details>
<summary>ESP RainMaker</summary>

* Requirements:
    * Latest ESP RainMaker app on an Apple Device (running at least iOS/iPadOS 16.4).
* Setup:
    * Open the ESP RainMaker app and select/create a group.
    * Click the `+` icon on top right.
    * Scan the QR code.
    * You might be asked to scan the QR code again or enter the setup code manually.
    * The device setup should start.
    * You might get a prompt that you are adding an `uncertified accessory`, click on `add anyway`.
    * In about 2 minutes, the app should show that the device has been added successfully.
* Control:
    * You should be able to control the device using the ESP RainMaker app.

</details>

<details>
<summary>Samsung</summary>

* Requirements:
    * Latest SmartThings app on an Android Device.
    * One of these <a href="https://support.smartthings.com/hc/en-us/articles/11219700390804-SmartThings-x-Matter-Integration-#" target="_blank">devices</a> already setup in your network:
        * Aeotec Smart Home Hub
        * SmartThings 2015 Hub (does not have Thread support)
        * SmartThings 2018 Hub
* Setup:
    * In the SmartThings app, go to devices.
    * Click the `+` icon, and select `QR code`.
    * Scan the QR code or enter the setup code manually.
    * The device setup should start.
    * You might get a prompt that this isn't a Matter certified device, click on `continue`.
    * You might also be prompted to enter your network credentials.
    * In about a minute, you will be prompted to add your device to any room and change the device name to what you want.
    * The app should show that the device has been added successfully.
* Control:
    * You should be able to control the device using the SmartThings app.

</details>

## Device Description

### Product

The device has 1 Socket.

1. Socket: On/Off
    * Default power state is on.
    * Power state on bootup is same as previous state.

### Device Drivers

The device has 1 LED, 1 Button and 1 Relay.

1. Button: GPIO
    * gpio_num: 9
2. Relay: GPIO
    * gpio_num: 4
3. LED: WS2812
    * ctrl_io: 8

## Device Functionality

### Indicators

The device has the following indicators:

#### Setup

* **Setup mode**: LED shows a breathing pattern continuously, about 2 seconds per cycle, with green color.
* **Setup started**: LED shows a breathing pattern continuously, about 1 second per cycle, with yellow color.
* **Setup complete**: LED shows the default state of the device and stops any ongoing patterns.
* **Setup failed**: LED remains solid, with red color.

#### Functional


#### Test Mode


#### Identification

* **Identification start**: LED blinks continuously, about 0 seconds per cycle, with red color.
* **Identification stop**: LED shows the default state of the device and stops any ongoing patterns.
* **Identification blink**: LED blinks for 1 second, about 1 second per cycle, with white color.
* **Identification breathe**: LED shows a breathing pattern for 15 seconds, about 1 second per cycle, with white color.
* **Identification okay**: LED blinks for 1 second, about 2 seconds per cycle, with green color.
* **Identification channel change**: LED blinks for 8 seconds, about 16 seconds per cycle, with yellow color.
* **Identification finish effect**: LED shows the default state of the device and stops any ongoing patterns.
* **Identification stop effect**: LED shows the default state of the device and stops any ongoing patterns.

### Factory Reset

To reset the device to factory settings:

* Press and hold the input button for about 5000 milliseconds. Then release the button to factory reset the device.
* The device will then reboot and enter setup mode.

### Test Mode

For device manufacturers, the firmware offers a test mode that can be used for testing of the device on the manufacturer's factory line. On first boot, the device will look for specific Wi-Fi/Thread networks in its vicinity to detect the kind of test actions to perform. These test actions are described below:

* **Test complete**: Mark test mode as complete. The device will not enter test mode ever again.
    * trigger: Default (Wi-Fi or Thread depending on the hardware)
    * ssid: test_complete
    * panid: 0x198F
    * mac: 1212121212121212
* **BLE MAC**: Broadcast the MAC address over BLE.
    * trigger: Default (Wi-Fi or Thread depending on the hardware)
    * ssid: test_ble_mac
    * panid: 0x198F
    * mac: 3434343434343434
* **Socket 1**: All sockets turn on and off, 3 times.
    * trigger: Default (Wi-Fi or Thread depending on the hardware)
    * ssid: test_socket_1
    * panid: 0x198F
    * mac: ABABABABABABABAB

<details>
<summary>Firmware from zip</summary>

## Firmware Flashing

If you are flashing the device firmware yourself from the downloaded zip, refer the appropriate section below for the flashing instructions. You can either use `esptool` in your `terminal/power_shell` or `Flash download tool`.

<details>
<summary>Using esptool on command line (MacOS, Linux, Windows)</summary>

### Pre-requisites

Install `esptool`
* Refer the <a href="https://docs.espressif.com/projects/esptool/en/latest/esp32/installation.html" target="_blank">installation</a>
* Open terminal/power_shell
* Run: `pip install esptool`
* Connect your device and note down the port
* You might have to provide the port in the below commands by passing `-p <port>`

### Flashing

Erase the flash first
* Run: `esptool.py erase_flash`

Flash the firmware
* 0x0: common_binaries/*_merged.bin
* Run: `esptool.py write_flash 0x0 common_binaries/*_merged.bin`

Flash other required files
* 0xD000: devices/\<number\>/esp_secure_cert.bin
* 0x1F2000: devices/\<number\>/fctry.bin
* Run: `esptool.py write_flash 0xD000 devices/1/esp_secure_cert.bin 0x1F2000 devices/1/fctry.bin`

Your device is now ready. You can go to the [Setup Code](#setup-code) section to continue setting up your device.

</details>

<details>
<summary>Using Flash Download Tool (Windows)</summary>

### Pre-requisites

Install the `Flash Download Tool`
* <a href="https://www.espressif.com/sites/default/files/tools/flash_download_tool_3.9.5.zip" target="_blank">Download</a> the tool
* Extract (unzip) the files and refer the pdf `doc/`
* Run the `flash_download_tool`
* Select your chip and proceed
* Connect your device and select the port in `COM`

### Flashing

Erase the flash first
* Click on `erase`

Flash the firmware
* 0x0: common_binaries/*_merged.bin
* Browse the file and enter its location and click on `start`

Flash other required files
* 0xD000: devices/\<number\>/esp_secure_cert.bin
* 0x1F2000: devices/\<number\>/fctry.bin
* Browse the files, enter their locations and click on `start`

Your device is now ready. You can go to the [Setup Code](#setup-code) section to continue setting up your device.

</details>
</details>

