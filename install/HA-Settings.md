# Some settings for Home Assistant
### (as an example)

In Terminal:
~~~
> ifconfig

wlan0:
   inet: 192.168.227.77
hassio:
   inet: 172.30.32.1
docker0:
   inet: 172.17.0.1
lo:
   inet: 127.0.0.1
~~~
In configuration.yaml:
~~~
homeassistant:
  auth_providers:
    - type: trusted_networks
      trusted_networks:
        - 192.168.227.0/24      #-- subnet of interface "wlan0"
        - 172.30.32.0/24        #-- subnet of interface "hassio"
        - 172.17.0.0/24         #-- subnet of interface "docker0"
        - 127.0.0.1             #-- IP address of interface "lo"
        - ::1                   #-- the same?
      allow_bypass_login: true
    - type: homeassistant
~~~
