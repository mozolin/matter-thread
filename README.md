URL: https://github.com/mozolin/matter-thread  

Powershell:  
~~~
systeminfo  
~~~
OS Name: Microsoft Windows 10 Pro  
OS Version: 10.0.19045 N/A Build 19045  
! Works successfully under this version of Windows!  
~~~  
systeminfo  
~~~
OS Name: Microsoft Windows 10 Enterprise  
OS Version: 10.0.19044 N/A Build 19044  
! Works without COM-ports sharing under this version of Windows!  
  
  
1) How to install Linux on Windows with WSL  
https://learn.microsoft.com/en-us/windows/wsl/install  
Check Ubuntu version:  
~~~
lsb_release -a  
~~~
If asked to add a new user, just do it, but after that it should switch to root.  
Powershell:  
~~~
ubuntu config --default-user root  
~~~
  
2) Ubuntu on Windows App Store  
https://apps.microsoft.com/search?query=ubuntu&hl=en-us&gl=US  
  
3) Install usbipd-win  
https://github.com/dorssel/usbipd-win/releases  
OR  
usbipd-win in WSL  
https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/additionalfeatures/wsl.html#usbipd-win-in-wsl  
Powershell:  
~~~
usbipd list -u  
~~~
!!! MAKE A PICTURE AT HOME !!!  
  
Ubuntu:  
~~~
lsusb  
~~~
!!! MAKE A PICTURE AT HOME !!!  
  
4) ESP-IDF Prerequisites  
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html  
https://wiki.seeedstudio.com/xiao_idf/  
To /root/.bashrc add:  
~~~
#-- Alias for setting up the ESP-IDF environment  
alias get_idf='. ~/esp-idf/export.sh'  
~~~
Than run:  
~~~
source ~/.bashrc  
~~~
  
5) ESP-IDF Setup  
https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html  
  
6) Matter Prerequisites  
https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/BUILDING.md#prerequisites  
  
7) ESP Matter Setup  
https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html#esp-matter-setup  
https://wiki.seeedstudio.com/xiao_esp32_matter_env/  
To /root/.bashrc add:  
~~~
#-- Alias for setting up the ESP-Matter environment  
alias get_matter='. ~/esp-matter/export.sh'  
  
#-- Enable ccache to speed up compilation  
alias set_cache='export IDF_CCACHE_ENABLE=1'  
~~~
Than run:  
~~~
source ~/.bashrc  
~~~
  
Note:  
"A complete installation of Ubuntu, ESP-IDF and ESP-Matter takes up about 20 GB of disk space on drive C."  
  
8) Install Visual Studio Code  
https://code.visualstudio.com/  
  
9) Install Remote WSL extension in Visual Studio Code  
https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/additionalfeatures/wsl.html#install-remote-wsl-extension-in-visual-studio-code  
  
10) ??? Test Setup (CHIP Tool)  
https://docs.espressif.com/projects/esp-matter/en/latest/esp32c6/developing.html#test-setup-chip-tool  
  
11) ??? Working with the CHIP Tool  
https://github.com/project-chip/connectedhomeip/blob/master/docs/development_controllers/chip-tool/chip_tool_guide.md  
  
12) Matter Shell Reference  
https://project-chip.github.io/connectedhomeip-doc/examples/chef/README_SHELL.html  
https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html#device-console  
  
13) How to generate Matter Onboarding Codes (QR Code and Manual Pairing Code)  
https://docs.espressif.com/projects/esp-matter/en/latest/esp32/faq.html#a1-9-how-to-generate-matter-onboarding-codes-qr-code-and-manual-pairing-code  
  
  
  
ERRORS:  
~~~  
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git  
export PATH="${PWD}/depot_tools:${PATH}"  
~~~  
