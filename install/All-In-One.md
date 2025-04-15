# Install Home Assistant

- On [Raspberry Pi 4b+](HA-Ubuntu22Desktop_RP4.md)  
- On [PC](HA-Ubuntu22Desktop_PC.md)  

# Install Portainer
DO NOT INSTALL PORTAINER! THERE WILL BE PROBLEMS WITH SUPERVISED HOME ASSISTANT!  

# Install ESP-IDF
~~~
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

git clone -b v5.4.1 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
source ./export.sh
~~~

# Install ESP-MATTER
~~~
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev default-jre
sudo apt-get install libsdl2-dev
sudo apt-get install mc
sudo apt-get install pi-bluetooth avahi-utils

cd esp-idf
source ./export.sh
cd ..

git clone --recursive https://github.com/espressif/esp-matter.git
cd esp-matter
./install.sh
cd ..
~~~
If bugs occured:
~~~
cd ~/esp-matter/connectedhomeip/connectedhomeip
git clean -Xdf
source ./scripts/bootstrap.sh
~~~

.bashrc:
~~~
#-- Alias for setting up the ESP-IDF environment
alias get_idf='. ~/esp-idf/export.sh'

#-- Alias for setting up the ESP-Matter environment
alias get_matter='. ~/esp-matter/export.sh'

#-- Alias for setting up the ConnectedHomeIP environment
alias get_chip='cd ~/esp-matter/connectedhomeip/connectedhomeip;. scripts/activate.sh'

#-- Enable ccache to speed up compilation (green color output)
green=$'\033[92m'
reset=$'\033[0m'
alias set_cache='echo $green; echo -e "Enable Ccache to speed up IDF compilation!\n"; echo $reset; export IDF_CCACHE_ENABLE=1'

#-- Alias for setting up all necessary environments
alias set_env='set savePath=$PWD; get_idf; get_matter; get_chip; set_cache; cd $savePath'
~~~
Launch ESP environment:
~~~
set_env
~~~

# Install ESP Thread BR
~~~
git clone --recursive https://github.com/espressif/esp-thread-br.git
~~~


# Install ChipTool
~~~
sudo apt update
sudo apt install snapd
sudo snap install chip-tool
~~~

# Install Samba
See [here](samba.md)


# Install Wake-on-LAN
~~~
sudo apt install ethtool
ifconfig
~~~
> **enp1s0**: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500  
>         inet 192.168.31.198  netmask 255.255.255.0  broadcast 192.168.31.255  
>         inet6 fe80::6986:d3e8:5cb6:9780  prefixlen 64  scopeid 0x20<link>  
>         ether **68:1d:ef:46:7c:45**  txqueuelen 1000  (Ethernet)  
  
~~~
sudo ethtool enp1s0 | grep "Wake-on"
~~~
> Supports Wake-on: pumbg  
> Wake-on: d (disabled)  
  
~~~
sudo ethtool --change enp1s0 wol g
sudo ethtool enp1s0 | grep "Wake-on"
~~~
> Supports Wake-on: pumbg  
> Wake-on: **g** (Wake on MagicPacket)  
  
~~~
nmcli con show
~~~
> NAME               UUID                                  TYPE      DEVICE  
> **Supervisor enp1s0**  be397120-9171-304c-817e-e084eb50825b  ethernet  enp1s0  
> Supervisor wlp2s0  ef23d080-c905-405a-98b3-1e85de873c84  wifi      wlp2s0  
  
~~~
sudo nmcli c modify "Supervisor enp1s0" 802-3-ethernet.wake-on-lan magic
nmcli c show "Supervisor enp1s0" | grep 802-3-eth
~~~
> 802-3-ethernet.wake-on-lan:             magic  
  
Wake up by sending a magic packet to the MAC ***68:1d:ef:46:7c:45***  
![](../images/ha/wol.png)  

We can also use apps to send a magic packet:
![](../images/ha/wol_app_01.jpg)  
  
![](../images/ha/wol_app_02.jpg)  
  
  

# Shutdown Ubuntu
Create a file shutdown.txt:  
~~~
shutdown -h now  
~~~
Run a Windows script:  
~~~
putty.exe -m shutdown.txt -ssh -P {port} -l {user} -pw {password} {ipaddress}
~~~
- {port} : SSH port
- {user} : user login
- {password} : user password
- {ipaddress} : server IP address

Install SSHPASS:
~~~
sudo apt-get install sshpass
~~~
Set automatic password input:
~~~
sshpass -p "{password}" ssh {user}@{ipaddress}
~~~
The shutdown command may be as follows:
~~~
ssh {user}@{ipaddress} -p {port} poweroff
~~~
