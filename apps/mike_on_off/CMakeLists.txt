cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED ENV{ESP_MATTER_PATH})
    message(FATAL_ERROR "Please set ESP_MATTER_PATH to the path of esp-matter repo")
endif(NOT DEFINED ENV{ESP_MATTER_PATH})

set(ESP_MATTER_PATH $ENV{ESP_MATTER_PATH})
set(MATTER_SDK_PATH ${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${ESP_MATTER_PATH}/examples/common/cmake_common/components_include.cmake)

set(EXTRA_COMPONENT_DIRS
    "${MATTER_SDK_PATH}/config/esp32/components"
    "${ESP_MATTER_PATH}/components"
    "${ESP_MATTER_PATH}/examples/common"
    ${extra_components_dirs_append})


#-- BasicInformationCluster (ClusterId 40, 0x0028), Endpoint 0
#-- AttributeId 1: VendorName
set(DEVICE_VENDOR_NAME "Mike_Home")
#-- AttributeId 3: ProductName
set(DEVICE_PRODUCT_NAME "Mike_On_Off")
#-- AttributeId 5: NodeLabel
set(DEVICE_NODE_LABEL "8_Relays_ESP32H2")
#-- AttributeId 6: Location
set(DEVICE_LOCATION "RU")
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

idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H;-Wno-overloaded-virtual" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security" APPEND)
