
# OTBR Web Page Optimization
https://github.com/espressif/esp-thread-br.git  
  
Considering that the ESP32-S3 chip on the ESP OTBR board has 16MB of flash memory, we can optimize the WEP page code inside the OTBR.  
  
### /examples/basic_thread_border_router/partitions.csv
Increase web_storage from *100K* to **1600K**
~~~
nvs,        data, nvs,      , 0x6000,
otadata,    data, ota,      , 0x2000,
phy_init,   data, phy,      , 0x1000,
ota_0,      app,  ota_0,    , 1600K,
ota_1,      app,  ota_1,    , 1600K,
web_storage,data, spiffs,   , 1600K,
rcp_fw,     data, spiffs,   , 640K,
~~~

### /examples/basic_thread_border_router/sdkconfig.defaults
Change *ESPTOOLPY_FLASHSIZE_4MB* to **ESPTOOLPY_FLASHSIZE_16MB**
~~~
CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y
~~~
  
The original HTML includes 3 links:
~~~
<link href="https://cdn.jsdelivr.net/npm/bootstrap@3.3.7/dist/css/bootstrap.min.css" rel="stylesheet">
<script src="https://code.jquery.com/jquery-1.10.2.min.js"></script>
<script src="https://d3js.org/d3.v3.min.js"></script>
~~~

### /components/esp_ot_br_server/frontend/static/static.min.css
Download *https://cdn.jsdelivr.net/npm/bootstrap@3.3.7/dist/css/bootstrap.min.css* and put its code into the **static.min.css** file.

### /components/esp_ot_br_server/frontend/static/static.min.js
Download *https://code.jquery.com/jquery-1.10.2.min.js* and *https://d3js.org/d3.v3.min.js* and put their code into the **static.min.js** file.

### /components/esp_ot_br_server/src/esp_br_web.c
Add a few lines to the *esp_br_web.c*:
~~~
...
    } else if (strcmp(info.file_name, "/static/static.min.css") == 0) {
        return style_css_get_handler(req, info.file_path);
    } else if (strcmp(info.file_name, "/static/static.min.js") == 0) {
        return script_js_get_handler(req, info.file_path);
...
~~~

### Minify code
We can also minify *index.html* (to **index.min.html**), *restful.html* (to **restful.min.html**) and *style.css* (to **style.min.css**) using the *minify* PHP-script:
Add new lines to the *esp_br_web.c* file:
~~~
...
    } else if (strcmp(info.file_name, "/index.min.html") == 0) {
        return index_html_get_handler(req, info.file_path);
    } else if (strcmp(info.file_name, "/static/style.min.css") == 0) {
        return style_css_get_handler(req, info.file_path);
    } else if (strcmp(info.file_name, "/static/restful.min.js") == 0) {
        return script_js_get_handler(req, info.file_path);
...
~~~

### Finish
After that, we need to compile and flash the firmware to get its current version!
