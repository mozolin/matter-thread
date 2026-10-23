
# Using OT-CTL commands in the Home Assistant terminal

>  
> docker ps | grep -i openthread  
>  
~~~
aea015e5d46d   homeassistant/aarch64-addon-otbr:3.3.0   "/init"   app_core_openthread_border_router
~~~

>  
> docker exec -it app_core_openthread_border_router ot-ctl
>  
    
>  
> \> router table
>  
~~~
| ID | RLOC16 | Next Hop | Path Cost | LQ In | LQ Out | Age | Extended MAC     | Link |
+----+--------+----------+-----------+-------+--------+-----+------------------+------+
|  0 | 0x0000 |       10 |         1 |     3 |      3 |  14 | 06aa461d666099b8 |    1 |
|  6 | 0x1800 |       31 |         1 |     3 |      3 |   4 | feac65e73fa342a4 |    1 |
|  7 | 0x1c00 |       10 |         1 |     3 |      3 |  13 | 7a5f30a5811f23f4 |    1 |
| 10 | 0x2800 |        0 |         1 |     3 |      3 |  18 | f6b24e113ba5bc96 |    1 |
| 31 | 0x7c00 |       10 |         1 |     3 |      3 |   8 | ba99d87881c9627d |    1 |
| 33 | 0x8400 |       63 |         0 |     0 |      0 |   0 | da89fc5f54608a76 |    0 |
| 45 | 0xb400 |       10 |         1 |     3 |      3 |   9 | d29e0f65f35482e2 |    1 |
| 50 | 0xc800 |        0 |         1 |     3 |      3 |  21 | 6ad3658ae267d125 |    1 |
~~~
  
>  
> \> neighbor table
>  
~~~
| Role | RLOC16 | Age | Avg RSSI | Last RSSI | LQ In |R|D|N| Extended MAC     | Version |
+------+--------+-----+----------+-----------+-------+-+-+-+------------------+---------+
|   R  | 0x0000 |  29 |      -49 |       -49 |     3 |1|1|1| 06aa461d666099b8 |       5 |
|   R  | 0x1800 |  28 |      -42 |       -43 |     3 |1|1|1| feac65e73fa342a4 |       5 |
|   R  | 0x1c00 |  22 |      -44 |       -45 |     3 |1|1|1| 7a5f30a5811f23f4 |       5 |
|   R  | 0x2800 |  12 |      -40 |       -58 |     3 |1|1|1| f6b24e113ba5bc96 |       5 |
|   R  | 0x7c00 |  20 |      -43 |       -44 |     3 |1|1|1| ba99d87881c9627d |       5 |
|   R  | 0xb400 |  21 |      -39 |       -40 |     3 |1|1|1| d29e0f65f35482e2 |       5 |
|   R  | 0xc800 |   2 |      -20 |       -20 |     3 |1|1|1| 6ad3658ae267d125 |       5 |
~~~
  
>  
> \> child table  
>  
~~~
| ID  | RLOC16 | Timeout    | Age        | LQ In | C_VN |R|D|N|Ver|CSL|QMsgCnt|Suprvsn| Extended MAC     |
+-----+--------+------------+------------+-------+------+-+-+-+---+---+-------+-------+------------------+
~~~
  
>  
> \> meshdiag topology  
>  
~~~
id:33 rloc16:0x8400 ext-addr:da89fc5f54608a76 ver:5 - me - br
    3-links:{ 00 06 07 10 31 45 50 }
id:06 rloc16:0x1800 ext-addr:feac65e73fa342a4 ver:5
    3-links:{ 00 07 31 33 45 50 }
id:00 rloc16:0x0000 ext-addr:06aa461d666099b8 ver:5 - leader - br
    3-links:{ 06 07 10 31 33 45 50 }
id:31 rloc16:0x7c00 ext-addr:ba99d87881c9627d ver:5 - br
    3-links:{ 00 06 07 10 33 45 50 }
id:50 rloc16:0xc800 ext-addr:6ad3658ae267d125 ver:5
    3-links:{ 00 06 07 10 31 33 45 }
id:45 rloc16:0xb400 ext-addr:d29e0f65f35482e2 ver:5 - br
    3-links:{ 00 06 07 10 31 33 50 }
id:07 rloc16:0x1c00 ext-addr:7a5f30a5811f23f4 ver:5 - br
    3-links:{ 00 06 10 31 33 45 50 }
id:10 rloc16:0x2800 ext-addr:f6b24e113ba5bc96 ver:5 - br
    3-links:{ 00 06 07 31 33 45 50 }
~~~

