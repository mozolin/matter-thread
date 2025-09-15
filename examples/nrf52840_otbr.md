
# Documentation
**1) OpenThread Border Router**  
[https://openthread.io/guides/border-router](https://openthread.io/guides/border-router)  
Used:  
- Raspberry Pi (Thread border router)  
- nRF52840 USB Dongle (RCP)  
- nRF52840 USB Dongle (End device)  
  
**2) Build a Thread network with nRF52840 boards and OpenThread**  
[https://openthread.io/codelabs/openthread-hardware#1](https://openthread.io/codelabs/openthread-hardware#1)  
Used:  
- x86-based Linux (Ubuntu) machine (to serve as the host to a RCP and to flash all nRF52840 PDK boards)
- nRF52840 PDK boards (RCP and End devices)
  
DOwnloads:  
- Download J-Link Software and Documentation Pack: JLink_Linux_V864_x86_64.deb
- Download nRF5x Command Line Tools: nrf-command-line-tools_10.24.2_amd64.deb
- Download ARM GNU Toolchain portable archive: gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2


  
**3) Configuring a radio co-processor**  
[https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/protocols/thread/tools.html#configuring_a_radio_co-processor](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/protocols/thread/tools.html#configuring_a_radio_co-processor)  
  
Install WEST:  
[https://docs.zephyrproject.org/latest/develop/west/install.html](https://docs.zephyrproject.org/latest/develop/west/install.html)  
~~~
pip3 install --user -U west
~~~
It will be installed in the *~/.local/bin* folder, so we should check for it in *.profile*:  
~~~
if [ -d "$HOME/.local/bin" ] ; then
    PATH="$HOME/.local/bin:$PATH"
fi
~~~

  
**4) How to Setup a OpenThread Border Router Using a NRF52840 USB Stick and Connect the Thread Sensor Tag**  
[https://www.instructables.com/How-to-Setup-a-OpenThread-Border-Router-Using-a-NR/](https://www.instructables.com/How-to-Setup-a-OpenThread-Border-Router-Using-a-NR/)  
[Local copy (.pdf)](nrf52840/docs/How_to_setup_a_OTBR_using_a_NRF52840_USB_stick_and_connect_the_Thread_Sensor_Tag.pdf)  
Used:  
- Raspberry Pi 4 (Thread border router)  
- nRF52840 USB Dongle (RCP)  
- Thread Sensor Tag by open-things (End device)  

**5) Open Thread Border Router on Linux Ubuntu**  
*October 7, 2022*  
[https://www.hackster.io/skruglewicz/open-thread-border-router-on-linux-ubuntu-3d93d8](https://www.hackster.io/skruglewicz/open-thread-border-router-on-linux-ubuntu-3d93d8)  
Used:  
- Nordic Semiconductor nRF52840 Dongle
- Ubuntu


# Software
[https://disk.yandex.ru/d/-m2amEJlKxgsGw](https://disk.yandex.ru/d/-m2amEJlKxgsGw)  
- gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2  
- JLink_Linux_V864_x86_64.deb  
- nrf-command-line-tools_10.24.2_amd64.deb  
- nrfconnect-5.2.0-x86_64.AppImage  


# Firmware
OpenThread nRF52840 Firmware Builder:  
[https://github.com/ArthFink/nrf52840-OpenThread](https://github.com/ArthFink/nrf52840-OpenThread)  
1) Go to the "Actions" tab of this repository.  
2) Select the latest workflow run for the "Bimonthly Release" workflow.  
3) Select the "release" section  
4) Go to the "Create GitHub release with .bin files" link  
5) Click on the "Release ready at ..." link  
6) Choose appropriate binary ([ot-cli-ftd-USB.hex](nrf52840/firmware/ot-cli-ftd-USB.hex) - for instance)  
  
![](nrf52840/firmware/nrf52840-OpenThread-rcp.png)  
