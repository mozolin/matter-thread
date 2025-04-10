
# Parse Thread Network info

Get list of routers from Thread BR CLI 
~~~
> router table

| ID | RLOC16 | Next Hop | Path Cost | LQ In | LQ Out | Age | Extended MAC     | Link |
+----+--------+----------+-----------+-------+--------+-----+------------------+------+
|  2 | 0x0800 |       12 |         1 |     3 |      3 |   7 | ee87506bb564af15 |    1 |
| 12 | 0x3000 |        2 |         1 |     3 |      3 |   6 | f6f7117a6cbc2cf1 |    1 |
| 17 | 0x4400 |       63 |         0 |     0 |      0 |   0 | 7a64fd2b41e75415 |    0 |
~~~

## OTBR (RLOC16 = 0x4400, ExtMAC = 7a64fd2b41e75415)
hex: 0x4400, dec: 17408, bin: 0100010000000000  
RouterId: 010001 = 17 (0x11), ChildId: 000000000 = 0  
~~~
ipaddr
~~~
> fdec:29c2:f04b:4b23:0:ff:fe00:fc11  
> fd8a:fd34:7273:1:efee:6880:1fee:630c  
> fdec:29c2:f04b:4b23:0:ff:fe00:fc10  
> fdec:29c2:f04b:4b23:0:ff:fe00:fc38  
> fdec:29c2:f04b:4b23:0:ff:fe00:fc00  
> fdec:29c2:f04b:4b23:fc4b:9b0d:2fb1:3cea  
> fe80:0:0:0:7864:fd2b:41e7:5415
> **fdec:29c2:f04b:4b23:0:ff:fe00:4400**
- mesh-loc-pref = fdec:29c2:f04b:4b23
- iid = 0:ff:fe00
- rloc16 = 4400


## End Device OT_CLI (RLOC16 = 0x0800, ExtMAC = ee87506bb564af15)
Example: https://github.com/espressif/esp-idf/tree/master/examples/openthread/ot_cli  
hex: 0x0800, dec: 2048, bin: 0000100000000000  
RouterId: 000010 = 2 (0x02), ChildId: 000000000 = 0  
~~~
ipaddr
~~~
> fdec:29c2:f04b:4b23:0:ff:fe00:800  
> fd8a:fd34:7273:1:3353:21d2:e7ae:bd73  
> fdec:29c2:f04b:4b23:2c3e:e917:3b28:ddf  
> fe80:0:0:0:ec87:506b:b564:af15  


## End Device MATTER-LIGHT (RLOC16 = 0x3000, ExtMAC = f6f7117a6cbc2cf1)
Example: https://github.com/espressif/esp-matter/tree/main/examples/light  
hex: 0x3000, dec: 12288, bin: 0011000000000000  
RouterId: 001100 = 12 (0x0c), ChildId: 000000000 = 0  
~~~
ipaddr
~~~
> fdec:29c2:f04b:4b23:0:ff:fe00:3000  
> fd8a:fd34:7273:1:7507:5977:7c50:7851  
> fdec:29c2:f04b:4b23:6d20:a1a9:cfcc:367b  
> fe80:0:0:0:f4f7:117a:6cbc:2cf1  

Get list of Matter devices (ServiceType = _matter._tcp)  
~~~
> sudo avahi-browse _matter._tcp

+ hassio  IPv4  663D59C323043403-0000000000000001  _matter._tcp  local
+ hassio  IPv4  64AEABD33B7DC79F-00000000000004D2  _matter._tcp  local
+ wlan0   IPv4  663D59C323043403-0000000000000001  _matter._tcp  local
+ wlan0   IPv6  663D59C323043403-0000000000000001  _matter._tcp  local
+ wlan0   IPv4  64AEABD33B7DC79F-00000000000004D2  _matter._tcp  local
+ wlan0   IPv6  64AEABD33B7DC79F-00000000000004D2  _matter._tcp  local
~~~
Instances:  
663D59C323043403-0000000000000001 - OTBR  
64AEABD33B7DC79F-00000000000004D2 - EndDevice, NodeId = 1234 (0x04D2)  

~~~
> mdns t

2025/04/09 17:30:07 &{{_smb._tcp.local _services._dns-sd._udp local _services._dns-sd._udp.local. _smb._tcp.local._services._dns-sd._udp.local. _services._dns-sd._udp.local.}  0 [] 4500 [] []}
2025/04/09 17:30:07 &{{_device-info._tcp.local _services._dns-sd._udp local _services._dns-sd._udp.local. _device-info._tcp.local._services._dns-sd._udp.local. _services._dns-sd._udp.local.}  0 [] 4500 [] []}
2025/04/09 17:30:08 &{{_meshcop._udp.local _services._dns-sd._udp local _services._dns-sd._udp.local. _meshcop._udp.local._services._dns-sd._udp.local. _services._dns-sd._udp.local.}  0 [] 4500 [] []}
2025/04/09 17:30:08 &{{_matter._tcp.local _services._dns-sd._udp local _services._dns-sd._udp.local. _matter._tcp.local._services._dns-sd._udp.local. _services._dns-sd._udp.local.}  0 [] 4500 [] []}
2025/04/09 17:30:08 &{{_home-assistant._tcp.local _services._dns-sd._udp local _services._dns-sd._udp.local. _home-assistant._tcp.local._services._dns-sd._udp.local. _services._dns-sd._udp.local.}  0 [] 4500 [] []}
~~~
