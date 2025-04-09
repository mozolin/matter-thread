  
# ESP Thread BR WEB-GUI
https://docs.espressif.com/projects/esp-thread-br/en/latest/codelab/web-gui.html  
  
## Thread REST APIs
https://github.com/espressif/esp-thread-br/blob/main/components/esp_ot_br_server/src/openapi.yaml  
  
From the border router command line interface:  
~~~
router table
~~~
> | ID | RLOC16 | Next Hop | Path Cost | LQ In | LQ Out | Age | Extended MAC     | Link |  
> +----+--------+----------+-----------+-------+--------+-----+------------------+------+  
> |  7 | 0x1c00 |       38 |         1 |     3 |      3 |  14 | 3e8a175887dbb4e7 |    1 |  
> | 17 | 0x4400 |       63 |         0 |     0 |      0 |   0 | 7a64fd2b41e75415 |    0 |  
> | 38 | 0x9800 |        7 |         1 |     3 |      3 |   0 | 1e1b2055963191b3 |    1 |  
  
If, for example, the IP address of the Thread Border web interface is 192.168.31.157, we can use its REST API to get some information.  
~~~
http:/192.168.31.157/diagnostics
~~~
[diagnostics.json](esp-thread-br-web-gui/diagnostics.json)  
~~~
http:/192.168.31.157/available_network
~~~
[available_network.json](esp-thread-br-web-gui/available_network.json)  
