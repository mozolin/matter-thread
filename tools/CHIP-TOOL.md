# Working with the CHIP Tool
+ https://github.com/project-chip/connectedhomeip/blob/master/docs/development_controllers/chip-tool/chip_tool_guide.md#installation  
+ https://project-chip.github.io/connectedhomeip-doc/development_controllers/chip-tool/chip_tool_guide.html#installation  
  
The CHIP Tool (chip-tool) is a Matter controller implementation that allows to commission a Matter device into the network and to communicate with it using Matter messages, which may encode Data Model actions, such as cluster commands.  
The tool also provides other utilities specific to Matter, such as parsing of the setup payload or performing discovery actions.  
  
On Linux distributions [running snapd](https://snapcraft.io/docs/installing-snapd), such as Ubuntu, the CHIP Tool can be installed using the [chip-tool snap](https://snapcraft.io/chip-tool). To do this, run:  
~~~
sudo snap install chip-tool
~~~
  
# [Working with the CHIP Tool](https://github.com/project-chip/connectedhomeip/blob/master/docs/development_controllers/chip-tool/chip_tool_guide.md#installation)
Another branch: https://project-chip.github.io/connectedhomeip-doc/development_controllers/chip-tool/chip_tool_guide.html#installation  
Examples: https://docs.silabs.com/matter/2.2.2/matter-wifi-getting-started-example/chip-tool-wifi
Examples: https://docs.espressif.com/projects/esp-matter/en/latest/esp32c6/developing.html#test-setup-chip-tool
# [Working with the CHIP Tool in WSL2](https://docs.silabs.com/matter/2.2.2/matter-wifi-getting-started-example/chip-tool-wifi)
ChipTool not working properly due to missing BLE adapter in WSL
~~~
chip-tool pairing code-thread 1 hex:0e080000000000010000000300001a4a0300001635060004001fffe002083dd5846a27dd139f0708fdec29c2f04b4b23051045005945ef9dbed88082d208673dad0f030f4f70656e5468726561642d3562393101025b9104109855950ef75071da53e996c50694576a0c0402a0f7f8 34970112332
~~~
> [DL] Disabling CHIPoBLE service due to error: BLE adapter unavailable
> [CTL] Commissioning discovery over BLE failed: BLE adapter unavailable


Example: [Basic Thread Border Router](../basic_thread_border_router.md)  
  
Building Android App:  
https://github.com/project-chip/connectedhomeip/blob/master/docs/platforms/android/android_building.md  
  