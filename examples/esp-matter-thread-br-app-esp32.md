# Matter ESP32 Thread Border Router Example
Github: https://github.com/project-chip/connectedhomeip/tree/master/examples/thread-br-app/esp32  

OpenThread Radio Co-Processor (RCP):  
~~~
cd ~/esp-idf/examples/openthread/ot_rcp
idf.py set-target esp32h2
idf.py build
~~~
Thread Border Router:
~~~
cd ~/esp-matter/connectedhomeip/connectedhomeip/examples/thread-br-app/esp32
idf.py set-target esp32s3
idf.py build
idf.py -p {port} erase-flash flash monitor
~~~
