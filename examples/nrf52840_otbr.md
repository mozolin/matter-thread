
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
  
**3) Configuring a radio co-processor**  
[https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/protocols/thread/tools.html#configuring_a_radio_co-processor](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/protocols/thread/tools.html#configuring_a_radio_co-processor)  
  
**4) How to Setup a OpenThread Border Router Using a NRF52840 USB Stick and Connect the Thread Sensor Tag**  
[https://www.instructables.com/How-to-Setup-a-OpenThread-Border-Router-Using-a-NR/](https://www.instructables.com/How-to-Setup-a-OpenThread-Border-Router-Using-a-NR/)  
[Local copy (.pdf)](nrf52840/docs/How_to_setup_a_OTBR_using_a_NRF52840_USB_stick_and_connect_the_Thread_Sensor_Tag.pdf)  
Used:  
- Raspberry Pi 4 (Thread border router)  
- nRF52840 USB Dongle (RCP)  
- Thread Sensor Tag by open-things (End device)  
  
# Software
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
