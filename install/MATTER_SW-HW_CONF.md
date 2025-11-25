# ESP-Matter: Changing the values of software and hardware configuration parameters
For Matter devices programmed with ESP-Matter, the default values are *ProductName*=**TEST_PRODUCT**, *VendorName*=**TEST_VENDOR** and *HardwareVersionString*=**TEST_VERSION**. So, distinguishing one device from another, for example, in Home Assistant, is quite difficult. This article describes ESP-Matter settings for changing these parameters.  
![](MATTER_SW-HW_CONF/sw-hw_conf_01.png)  
  
### 1. Example of a device with default settings:  
Default parameters in the description of the Matter node:  
![](MATTER_SW-HW_CONF/sw-hw_conf_02.png)  

Default parameters in the description of the BasicInformation cluster:  
![](MATTER_SW-HW_CONF/sw-hw_conf_03.png)  

Default contents of the CMakeLists.txt file:  
[sw-hw_conf_04-CMakeLists.txt](MATTER_SW-HW_CONF/sw-hw_conf_04-CMakeLists.txt)  
~~~
set(PROJECT_VER "1.0")
set(PROJECT_VER_NUMBER 1)

project(mike_on_off)
~~~
  
### 2. Example of a device with more correct settings:  
Changed parameters in the description of the Matter node:  
![](MATTER_SW-HW_CONF/sw-hw_conf_05.png)  

Add settings for **VendorName**, **ProductName**, and **HardwareVersionString** in the *CMakeLists.txt* file. The *PROJECT_VER* and *PROJECT_VER_NUMBER* parameters are used in ESP-Matter to set the values of **SoftwareVersionString** and **SoftwareVersion**, respectively. Change the *PROJECT_VER* value.  
[sw-hw_conf_06-CMakeLists.txt](MATTER_SW-HW_CONF/sw-hw_conf_06-CMakeLists.txt)  
~~~
#-- BasicInformationCluster (ClusterId 40, 0x0028), Endpoint 0
#-- AttributeId 1: VendorName
set(DEVICE_VENDOR_NAME "Mike_Home")
#-- AttributeId 3: ProductName
set(DEVICE_PRODUCT_NAME "Mike_On_Off")
#-- AttributeId 5: NodeLabel
set(DEVICE_NODE_LABEL "8_Relays_ESP32H2")
#-- AttributeId 6: Location
set(DEVICE_LOCATION "Home")
#-- AttributeId 7: HardwareVersion
set(CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION "See: sdkconfig.default")
#-- AttributeId 8: HardwareVersionString
set(DEVICE_HARDWARE_VERSION_STRING "2.2.2")
#-- AttributeId 9: SoftwareVersion
set(PROJECT_VER_NUMBER 1)
#-- AttributeId 10: SoftwareVersionString
set(PROJECT_VER "1.1.1")

#-- Define custom options for the compiler
add_compile_definitions(
    CONFIG_CUSTOM_DEVICE_NODE_LABEL="${DEVICE_NODE_LABEL}"
    CONFIG_CUSTOM_DEVICE_LOCATION="${DEVICE_LOCATION}"
)

#-- Set predefined options for the compiler
idf_build_set_property(COMPILE_OPTIONS "-DCHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME=\"${DEVICE_VENDOR_NAME}\"" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DCHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME=\"${DEVICE_PRODUCT_NAME}\"" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DCHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING=\"${DEVICE_HARDWARE_VERSION_STRING}\"" APPEND)

project(mike_on_off)
~~~
  
Add settings for **HardwareVersion** and **SoftwareVersion** to *sdkconfig.defaults* (they cannot be specified directly in *CMakeLists.txt*) and specify that the PROJECT_VER will be taken from the config:  
[sw-hw_conf_07-sdkconfig.defaults](MATTER_SW-HW_CONF/sw-hw_conf_07-sdkconfig.defaults)  
~~~
# Device Identification Options
CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION=123
CONFIG_DEVICE_SOFTWARE_VERSION_NUMBER=234

# Application manager
CONFIG_APP_PROJECT_VER_FROM_CONFIG=y
CONFIG_APP_PROJECT_VER="v666"
~~~

As we can see, **VendorName**, **ProductName**, and **HardwareVersionString** is correctly set from the *CMakeLists.txt* settings. The integer values of **HardwareVersion** and **SoftwareVersion** and the string value of **SoftwareVersionString**, is set from the *sdkconfig.defaults* settings  
![](MATTER_SW-HW_CONF/sw-hw_conf_08.png)  

For PROJECT_VER, disable usage from config:  
[sw-hw_conf_09-sdkconfig.defaults](MATTER_SW-HW_CONF/sw-hw_conf_09-sdkconfig.defaults)  
~~~
# Device Identification Options
CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION=123
CONFIG_DEVICE_SOFTWARE_VERSION_NUMBER=234

# Application manager
CONFIG_APP_PROJECT_VER_FROM_CONFIG=n
CONFIG_APP_PROJECT_VER="v666"
~~~

As we can see, the values of **SoftwareVersion** and **SoftwareVersionString** are now taken from the *CMakeLists.txt* settings  
![](MATTER_SW-HW_CONF/sw-hw_conf_10.png)  

The **Location** and **NodeLabel** parameters cannot be set in the project configuration, so we will change them programmatically:
~~~
void set_basic_attributes_esp_matter()
{
  uint16_t endpoint_id = 0x0000;

  //-- Set NodeLabel
  char node_label[] = CONFIG_CUSTOM_DEVICE_NODE_LABEL;
  esp_matter_attr_val_t node_label_val = esp_matter_char_str(node_label, strlen(node_label));
  esp_err_t err = esp_matter::attribute::update(
    endpoint_id,
    chip::app::Clusters::BasicInformation::Id,
    chip::app::Clusters::BasicInformation::Attributes::NodeLabel::Id,
    &node_label_val
  );
  
  //-- Set Location (unfortunately, it is not yet included in the cluster definition)
  char location[] = CONFIG_CUSTOM_DEVICE_LOCATION;
  esp_matter_attr_val_t location_val = esp_matter_char_str(location, strlen(location));
  err = esp_matter::attribute::update(
    endpoint_id,
    chip::app::Clusters::BasicInformation::Id,
    chip::app::Clusters::BasicInformation::Attributes::Location::Id,
    &location_val
  );
}
~~~
The values ​​for **CONFIG_CUSTOM_DEVICE_NODE_LABEL** and **CONFIG_CUSTOM_DEVICE_LOCATION** are defined in *CMakeLists.txt*.  
Unfortunately, the **Location** parameter is not yet included in the cluster definition, so we will save its processing for a future ESP-IDF/ESP-MATTER release.  
  
In summary, what we can see in Home Assistant:  
![](MATTER_SW-HW_CONF/sw-hw_conf_11.png)  
  
![](MATTER_SW-HW_CONF/sw-hw_conf_12.png)  
  
