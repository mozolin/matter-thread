# UBUNTU: SHOULD BE INSTALLED

## To execute SUDO WITHOUT PASSWORD and get user "mike" a root access
Add user to "sudo" group:  
~~~
sudo usermod -aG sudo mike
~~~
Open /etc/sudoers and add the line:  
> mike ALL=(ALL:ALL) NOPASSWD: ALL  
  
To check:
~~~
grep '^sudo:' /etc/group
sudo ls
~~~


## MICRO is a terminal-based text editor
https://github.com/zyedidia/micro  
https://forum.garudalinux.org/t/mastering-the-micro-text-editor/32889  
~~~
sudo apt install micro
~~~
To set Micro as default editor, edit/add the following in ~/.selected_editor (for both, the current user and root):  
> SELECTED_EDITOR="/bin/micro"

Install some plugins for Micro:  
https://micro-editor.github.io/plugins.html  
~~~
micro -plugin install editorconfig   # https://github.com/10sr/editorconfig-micro
micro -plugin install manipulator    # https://github.com/NicolaiSoeborg/manipulator-plugin
micro -plugin install autofmt        # https://github.com/a11ce/micro-autofmt
micro -plugin install filemanager    # ???
~~~
Settings in ~/.config/micro/settings.json
~~~
{
  "autosu": false,
  "colorcolumn": 80,
  "cursorline": true,
  "eofnewline": true,
  "ftoptions": true,
  "ignorecase": true,
  "indentchar": " ",
  "keepautoindent": true,
  "matchbrace": true,
  "ruler": true,
  "savecursor": true,
  "saveundo": true,
  "scrollbar": true,
  "scrollmargin": 3,
  "smartpaste": true,
  "softwrap": true,
  "splitbottom": true,
  "splitright": true,
  "status": true,
  "syntax": true,
  "tabmovement": true,
  "tabsize": 4,
  "tabstospaces": true
}
~~~
Bindings in ~/.config/micro/bindings.json:
~~~
{
  "Ctrl-f": "Find",
  "F3": "FindNext",
  ...
}
~~~

## XSCREENSAVER
https://www.debugpoint.com/install-change-autostart-setup-screensaver-ubuntu-linux/
Install
~~~
sudo apt install xscreensaver
~~~
Check
~~~
xscreensaver --help
~~~
Autostart: add an item to Startup Application:
> Name:    XScreenSaver
> Command: xscreensaver -nosplash
> Comment: Run XScreenSaver in the background
~~~
sudo apt remove gnome-screensaver
~~~

Disable Wayland: in /etc/gdm3/custom.conf set:  
> WaylandEnable=false

<!--
## Anbox?
https://github.com/anbox/anbox  
-->


## AVAHI + MDNS
How to use the Avahi mDNS/DNS-SD daemon:  
https://sleeplessbeastie.eu/2023/07/05/how-to-use-the-avahi-daemon/  
  
In case of error like "Not loading module atk-bridge":
~~~
apt purge --simulate libatk-adaptor (without sudo)
sudo apt purge libatk-adaptor
sudo reboot
sudo apt install libatk-adaptor
sudo reboot
~~~


## RUN .APPIMAGE
1) Making it executable:
~~~
chmod a+x exampleName.AppImage
~~~
2) Execute it (not as root!):
~~~
./exampleName.AppImage
~~~
3) if error "error loading libfuse.so.2" occured, install this lib:
~~~
sudo apt install libfuse2
~~~


## TERMINALS AND MONITORS
### Minicom
~~~
sudo apt install minicom
minicom -help
minicom -D /dev/ttyACM0
~~~
Exit from the terminal: Ctrl+A => X  
![](../images/system/minicom_exit.png)  

### Screen
~~~
sudo apt install screen
screen /dev/ttyACM0 115200
~~~
Exit from the terminal: Ctrl+A => D  
This is the most common way to exit a screen session while keeping its processes and applications running.  
  
Exit from the terminal: Ctrl+A => K  
This method closes the screen session and any processes running within it.  

### Cu
~~~
sudo apt install cu
cu -l /dev/ttyACM0 -s 115200
~~~
Exit from the terminal: type ~. (tilde + dot) and press Enter  

## CHECK SOME...

### Scan IPs
How to scan for IP Addresses on your Network:
~~~
arp -a
~~~
> ? (10.41.235.157) at 7c:df:a1:f3:56:58 [ether] on wlp1s0
> ? (10.41.235.87) at 1e:2b:1e:e3:39:ba [ether] on wlp1s0
> ? (172.30.32.2) at 9e:81:96:26:69:4a [ether] on hassio

### Screenshots
1) Press PrtScn and select a screen area, window and the entire screen
2) Press (not everywhere!) Win + right mouse button, select "Take Screenshot" - a copy of the entire screen

### Terminal Tabs
Ctrl+PgUp: Switch to the previous tab  
Ctrl+PgDn: Switch to the next tab  

### Language layout
Win+Space: Switch between languages  

### Remove DKMS (Dynamic Kernel Module Support) unremovable packages
~~~
sudo dkms status
~~~
> xyz12345/6.7.8: added
> xyz12345/6.7.8, 6.8.0-79-generic, x86_64: installed
~~~
sudo dkms remove xyz12345/6.7.8 --all
~~~
> Error!  
Just remove its folder:
~~~
sudo rm -r /var/lib/dkms/xyz12345_6.7.8
~~~

## RealTech 8812BU wireless lan 802.11ac USB NIC Driver
[https://github.com/morrownr/88x2bu-20210702](https://github.com/morrownr/88x2bu-20210702)  
### 1) Disable old driver
Disable "Realtek Semocondactor...Wireless" in the utility "Software & Updates": Do not use the device
### 2) Install new driver  
~~~
sudo apt update && sudo apt upgrade
sudo reboot

sudo apt install -y build-essential dkms git iw

mkdir -p ~/src
cd ~/src
git clone https://github.com/morrownr/88x2bu-20210702.git
cd ~/src/88x2bu-20210702
sudo sh install-driver.sh

sudo modprobe 88x2bu
~~~
  
### 3) Check installation  
  
~~~
lsmod | grep 88x2bu
~~~
  
If there is an error like "modprobe: ERROR: could not insert '88x2bu' key was rejected by service", we should disable SecureBoot in BIOS!  
Than check again:  
~~~
mokutil --sb-state
~~~
> SecureBoot disabled
~~~
lsmod | grep 88x2bu
~~~
> **88x2bu**     3903488  0  
> cfg80211   1363968  3 **88x2bu**,rtw88_core,mac80211  

~~~
nmcli
~~~
> wlx90de80a782e1: connecting (getting IP configuration) to Supervisor wlx90de80a782e1  
>         "Realtek RTL88x2bu"  
>         wifi (rtw_8822bu), 90:DE:80:A7:82:E1, hw, mtu 1500  
      
### 4) Edit config (/etc/modprobe.d/88x2bu.conf)
**IMPORTANT**: Prevent loading old driver rtw88 8822bu!
~~~
sudo ./edit-options.sh
~~~
> blacklist rtw88_8822bu  
> options 88x2bu rtw_switch_usb_mode=1 rtw_led_ctrl=1  
  

## Archives

### Create a .tar.gz archive of a folder
~~~
tar -czvf archive_name.tar.gz /path/to/your/folder
~~~

### Unpack a .tar.gz archive
~~~
tar -xvzf filename.tar.gz
~~~
