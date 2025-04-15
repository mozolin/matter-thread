
# Installing Home Assistant Supervised on Ubuntu
Complete manual: https://pimylifeup.com/ubuntu-home-assistant/  
PC with Ubuntu 22.04.5 LTS Desktop
~~~
sudo apt update
sudo apt upgrade -y

sudo apt install apparmor jq wget curl udisks2 libglib2.0-bin network-manager dbus lsb-release systemd-journal-remote binutils nfs-common rpcbind cifs-utils -y

curl -fsSL get.docker.com | sh
~~~

Get the latest version of os-agent from https://github.com/home-assistant/os-agent/releases
~~~
wget https://github.com/home-assistant/os-agent/releases/download/1.7.2/os-agent_1.7.2_linux_x86_64.deb

sudo dpkg -i os-agent_1.7.2_linux_x86_64.deb
~~~

Check an installation:
~~~
gdbus introspect --system --dest io.hass.os --object-path /io/hass/os
~~~

Get "homeassistant-supervised.deb" from https://github.com/home-assistant/supervised-installer/releases/, but only version 1.8.0 - newer versions cause supervisor to crash
~~~
sudo apt install grub2-common

sudo BYPASS_OS_CHECK=true dpkg -i --ignore-depends=systemd-resolved ./homeassistant-supervised.deb

sudo reboot
~~~
