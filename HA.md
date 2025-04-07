# Home Assistant Integration
https://docs.espressif.com/projects/esp-thread-br/en/latest/codelab/home_assistant.html  

## Installation (Ubuntu Desktop 22.04 on Raspberry Pi 4B+)
- RaspberryPi 4B+ flash with Ubuntu 22.04.5 LTS Desktop
- Prerequisites
~~~
apt install apparmor bluez cifs-utils curl dbus jq libglib2.0-bin lsb-release network-manager nfs-common systemd-journal-remote udisks2 wget -y
~~~
*The "systemd-resolved" should be removed from the default list - there is no package like this*
~~~
curl -fsSL get.docker.com | sh
~~~
- get the last version https://github.com/home-assistant/os-agent/releases like "os-agent_N.N.N_linux_aarch64.deb"
*Here: N.N.N = 1.7.2 (as an example)*
~~~
dpkg -i os-agent_1.7.2_linux_aarch64.deb
~~~
- get "homeassistant-supervised.deb" from https://github.com/home-assistant/supervised-installer/releases/, but ver 2.0.0
~~~
sudo BYPASS_OS_CHECK=true dpkg -i --ignore-depends=systemd-resolved ./homeassistant-supervised.deb
~~~
Run Home Assistant: http://localhost:8123/  

## Install Portainer
~~~
sudo docker pull portainer/portainer-ce:latest
sudo docker run -d -p 9000:9000 --name=portainer --restart=always -v /var/run/docker.sock:/var/run/docker.sock -v portainer_data:/data portainer/portainer-ce:latest
~~~
Run portainer: http://localhost:9000/  
  
![](images/ha/add_matter_device_01.jpg)  
![](images/ha/add_matter_device_02.jpg)  
![](images/ha/add_matter_device_03.jpg)  
![](images/ha/add_matter_device_04.jpg)  
![](images/ha/add_matter_device_05.jpg)  
![](images/ha/add_matter_device_06.jpg)  
![](images/ha/add_matter_device_07.jpg)  
![](images/ha/add_matter_device_08.jpg)  
![](images/ha/add_matter_device_09.jpg)  
![](images/ha/add_matter_device_10.jpg)  
![](images/ha/add_matter_device_11.jpg)  
![](images/ha/add_matter_device_12.jpg)  
![](images/ha/HA-ActiveDatasetTLVs.png)  
![](images/ha/HA_01.png)  
![](images/ha/HA_02.png)  
![](images/ha/HA_03.png)  
![](images/ha/HA_04.png)  
![](images/ha/HA_05.png)  
![](images/ha/HA_06.png)  
![](images/ha/HA_07.png)  
![](images/ha/HA_08.png)  
![](images/ha/HA_666_01.png)  
![](images/ha/HA_666_02.png)  
![](images/ha/HA_666_03.png)  
![](images/ha/HA_666_04.png)  
  