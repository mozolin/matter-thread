# HA: Get Matter/Thread entities list

## 1. matter_devices.py (Ubuntu)
Creates sensor "sensor.orphaned_matter_devices" to get entities without entity_id.  
- [matter_devices.py](python/ubuntu/home/mike/matter_devices.py)  
- [matter_devices.log](python/ubuntu/home/mike/matter_devices.txt)  
- [JSON sensor template](python/ubuntu/home/mike/matter_devices.txt.json)

## 2. cron (Ubuntu)
Reloads sensor "sensor.orphaned_matter_devices" every minute:  
- [crontab](python/ubuntu/crontab)  
  
It needs to be installed:
~~~
sudo pip3 install websocket-client
~~~

## 3. sensor.matter_devices (Home Assistant OS)
Gets all Matter entities list:  
- [sensor.matter_devices](python/hassio/config/entities/sensors/matter_devices.yaml)  
  
The value of the "devices" attribute of the "matter_devices" sensor can be as follows:
~~~
[
  {
    "name": "matter_plug_#4",
    "friendly_name": "Matter Plug #4",
    "id": "dec463061385fb78e87066f965613890",
    "manufacturer": "Quectel",
    "model": "Quectel_Matter_Product",
    "hw_version": "TEST_VERSION",
    "sw_version": "QUEC_1.0.1.0",
    "via_device_id": "",
    "entry_type": "",
    "configuration_url": ""
  },
  {
    "name": "mike_product",
    "friendly_name": "MIKE_PRODUCT",
    "id": "2c8d3a12c412abdab474689a092f8335",
    "manufacturer": "MIKE_VENDOR",
    "model": "MIKE_PRODUCT",
    "hw_version": "1.1.1",
    "sw_version": "2.2.2",
    "via_device_id": "",
    "entry_type": "",
    "configuration_url": ""
  }
]
~~~
