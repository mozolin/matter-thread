# Thread Border Router
Github: https://github.com/espressif/esp-matter/tree/main/examples/thread_border_router  
  
OpenThread Radio Co-Processor (RCP):  
~~~
cd ~/esp-idf/examples/openthread/ot_rcp
idf.py set-target esp32h2 build
~~~
Thread Border Router:
~~~
cd ~/esp-matter/examples/thread_border_router
idf.py set-target esp32s3 build
idf.py -p {port} erase-flash flash
~~~
  
