# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED ENV{ESP_MATTER_PATH})
    message(FATAL_ERROR "Please set ESP_MATTER_PATH to the path of esp-matter repo")
endif(NOT DEFINED ENV{ESP_MATTER_PATH})

if(NOT DEFINED ENV{ESP_MATTER_DEVICE_PATH})
    if("${IDF_TARGET}" STREQUAL "" OR "${IDF_TARGET}" STREQUAL "esp32" OR "${IDF_TARGET}" STREQUAL "esp32c3" OR "${IDF_TARGET}" STREQUAL "esp32s3")
        # use hollow device type because the iot button has a require of 'esp_adc_cal'
        # which is renamed to 'esp_adc' in IDF v5.0
        # TODO: set the ESP_MATTER_DEVICE_PATH to a certain target.
        set(ENV{ESP_MATTER_DEVICE_PATH} $ENV{ESP_MATTER_PATH}/device_hal/device/hollow)
    else()
        message(FATAL_ERROR "Unsupported IDF_TARGET")
    endif()
endif(NOT DEFINED ENV{ESP_MATTER_DEVICE_PATH})

set(ESP_MATTER_PATH $ENV{ESP_MATTER_PATH})
set(MATTER_SDK_PATH ${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip)

# This should be done before using the IDF_TARGET variable.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include($ENV{ESP_MATTER_DEVICE_PATH}/esp_matter_device.cmake)

set(EXTRA_COMPONENT_DIRS
    "${ESP_MATTER_PATH}/examples/common"
    "${MATTER_SDK_PATH}/config/esp32/components"
    "${ESP_MATTER_PATH}/components"
    "${ESP_MATTER_PATH}/device_hal/device"
    ${extra_components_dirs_append})

idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H;-fpermissive;-Wno-overloaded-virtual" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
# For RISCV chips, project_include.cmake sets -Wno-format, but does not clear various
# flags that depend on -Wformat
idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security;-Wno-error=cpp" APPEND)


set(DEVICE_VENDOR_NAME "Mike_Home")
set(DEVICE_PRODUCT_NAME "Mike_Zigbee_Bridge")
set(DEVICE_HARDWARE_VERSION_STRING "2.2.2")
set(PROJECT_VER "1.1.1")
set(PROJECT_VER_NUMBER 1)

idf_build_set_property(COMPILE_OPTIONS "-DCHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME=\"${DEVICE_VENDOR_NAME}\"" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DCHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME=\"${DEVICE_PRODUCT_NAME}\"" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DCHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING=\"${DEVICE_HARDWARE_VERSION_STRING}\"" APPEND)

project(zigbee_bridge)
