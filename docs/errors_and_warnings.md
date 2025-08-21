# Errors and Warnings

### "ets_delay_us in ROM, wait usb download"
In case when running ESP32 monitoring...  
~~~
idf.py -p /dev/ttyACM0 monitor
~~~
What to do?  
- Simply press the RESET (EN) button on the board. On the next startup attempt, the bootloader will wait a few seconds again, but if you don't start uploading, it will simply run the existing program.
- Disconnect and reconnect the power (USB cable).
