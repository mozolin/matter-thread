
# Parse Thread Network info

## Get list of routers from Thread BR CLI 
~~~
> router table

| ID | RLOC16 | Next Hop | Path Cost | LQ In | LQ Out | Age | Extended MAC     | Link |
+----+--------+----------+-----------+-------+--------+-----+------------------+------+
|  2 | 0x0800 |       12 |         1 |     3 |      3 |   7 | ee87506bb564af15 |    1 |
| 12 | 0x3000 |        2 |         1 |     3 |      3 |   6 | f6f7117a6cbc2cf1 |    1 |
| 17 | 0x4400 |       63 |         0 |     0 |      0 |   0 | 7a64fd2b41e75415 |    0 |
~~~
More details about RLOC16 can be found [here](../tools/rloc16.md)

## OTBR (RLOC16 = 0x4400, ExtMAC = 7a64fd2b41e75415)
hex: 0x4400, dec: 17408, bin: "010001 0 000000000"  
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
hex: 0x0800, dec: 2048, bin: "000010 0 000000000"  
RouterId: 000010 = 2 (0x02), ChildId: 000000000 = 0  
~~~
ipaddr
~~~
> fdec:29c2:f04b:4b23:0:ff:fe00:800  
> fd8a:fd34:7273:1:3353:21d2:e7ae:bd73  
> fdec:29c2:f04b:4b23:2c3e:e917:3b28:ddf  
> fe80:0:0:0:ec87:506b:b564:af15  
- mesh-loc-pref = fdec:29c2:f04b:4b23
- iid = 0:ff:fe00
- rloc16 = 800


## End Device MATTER-LIGHT (RLOC16 = 0x3000, ExtMAC = f6f7117a6cbc2cf1)
Example: https://github.com/espressif/esp-matter/tree/main/examples/light  
hex: 0x3000, dec: 12288, bin: "001100 0 000000000"  
RouterId: 001100 = 12 (0x0c), ChildId: 000000000 = 0  
~~~
ipaddr
~~~
> fdec:29c2:f04b:4b23:0:ff:fe00:3000  
> fd8a:fd34:7273:1:7507:5977:7c50:7851  
> fdec:29c2:f04b:4b23:6d20:a1a9:cfcc:367b  
> fe80:0:0:0:f4f7:117a:6cbc:2cf1  
- mesh-loc-pref = fdec:29c2:f04b:4b23
- iid = 0:ff:fe00
- rloc16 = 3000


## Get list of Matter devices (_matter._tcp service)  

~~~
> sudo avahi-browse _matter._tcp
> sudo avahi-browse _matter._tcp | grep -a "+ hassio" #-- hassio only

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


## Get information about the _meshcop._udp service

> sudo avahi-browse -r -t _meshcop._udp  
> sudo avahi-browse -t -p -r _meshcop._udp  
> sudo avahi-browse -t -p -r _meshcop._udp | grep -a "=;hassio"  
~~~
=;
hassio;             | interface
IPv4;               | protocol
esp-ot-br;          | instancename
_meshcop._udp;      | service
local;              | domain
esp-ot-br.local;    | hostname
192.168.31.157;     | address
49154;              | port
{see below}         | txt
"dn=DefaultDomain" "tv=1.4.0" "nn=OpenThread-5b91" "mn=BorderRouter" "vn=OpenThread" "rv=1"...
~~~


## Parse and assembly active dataset
Get active dataset in the OTRB console:
~~~
dataset active -x
~~~
> 0e080000000000010000000300001a4a0300001635060004001fffe002083dd5846a27dd139f0708fdec29c2f04b4b23051045005945ef9dbed88082d208673dad0f030f4f70656e5468726561642d3562393101025b9104109855950ef75071da53e996c50694576a0c0402a0f7f8  
  
Parse this dataset with [tlv-parser](../tools/tlv-parser)  
~~~
t: 14 (ACTIVETIMESTAMP), l: 8, v: 0x0000000000010000
t:  0 (CHANNEL), l: 3, v: 0x00001a
t: 74 (APPLE_TAG_UNKNOWN), l: 3, v: 0x000016
t: 53 (CHANNELMASK), l: 6, v: 0x0004001fffe0
t:  2 (EXTPANID), l: 8, v: 0x3dd5846a27dd139f
t:  7 (MESHLOCALPREFIX), l: 8, v: 0xfdec29c2f04b4b23
t:  5 (NETWORKKEY), l: 16, v: 0x45005945ef9dbed88082d208673dad0f
t:  3 (NETWORKNAME), l: 15, v: b'OpenThread-5b91'
t:  1 (PANID), l: 2, v: 0x5b91
t:  4 (PSKC), l: 16, v: 0x9855950ef75071da53e996c50694576a
t: 12 (SECURITYPOLICY), l: 4, v: 0x02a0f7f8
~~~

## Generate and parse Matter pairing codes
Generate and parse Matter pairing codes with [setup-payload](../tools/setup-payload)  
