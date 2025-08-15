# UBUNTU: SHOULD BE INSTALLED

### To execute sudo without password and get user "mike" a root access
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


### Micro is a terminal-based text editor
https://github.com/zyedidia/micro  
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

### XScreenSaver
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
