# Install Samba (Ubuntu Desktop 22.04 on Raspberry Pi 4B+)
~~~
sudo apt install samba -y
whereis samba
~~~
Put into /etc/samba/smb.conf:  
> [esp]  
>    comment = ESP-IDF Samba  
>    path = /home/pi  
>    public = yes  
>    writable = yes  
>    read only = no  
>    guest ok = yes  
>    create mask = 0775  
>    directory mask = 0775  
>    force create mode = 0775  
>    force directory mode = 0775  
  
~~~
sudo service smbd restart
sudo ufw allow samba
~~~
Add user as a Samba user
~~~
sudo smbpasswd -a pi
~~~
*Enter password:* **raspberry**  
*Re-enter password:* **raspberry**  
  
Autostart:  
~~~
sudo systemctl enable smbd
~~~

On Windows, open up File Manager and edit the file path to:  
> \\\ip-address\esp
