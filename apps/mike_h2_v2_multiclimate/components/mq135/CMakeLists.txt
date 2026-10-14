set(component_srcs "mq135_driver_3v3.c" "mq135_driver.c")

#if(CONFIG_MQ135_3V3_CIRCUIT)
#    list(APPEND component_srcs "mq135_driver_3v3.c")
#else()
#    list(APPEND component_srcs "mq135_driver.c")
#endif()

idf_component_register(
    SRCS "${component_srcs}"
    PRIV_REQUIRES driver
    INCLUDE_DIRS "."
    REQUIRES esp_adc
)
