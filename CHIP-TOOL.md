# Working with the CHIP Tool
+ https://github.com/project-chip/connectedhomeip/blob/master/docs/development_controllers/chip-tool/chip_tool_guide.md#installation  
+ https://project-chip.github.io/connectedhomeip-doc/development_controllers/chip-tool/chip_tool_guide.html#installation  
  
The CHIP Tool (chip-tool) is a Matter controller implementation that allows to commission a Matter device into the network and to communicate with it using Matter messages, which may encode Data Model actions, such as cluster commands.  
The tool also provides other utilities specific to Matter, such as parsing of the setup payload or performing discovery actions.  
  
On Linux distributions [running snapd](https://snapcraft.io/docs/installing-snapd), such as Ubuntu, the CHIP Tool can be installed using the [chip-tool snap](https://snapcraft.io/chip-tool). To do this, run:  
~~~
sudo snap install chip-tool
~~~
  
Example: [Basic Thread Border Router](basic_thread_border_router.md)  
  