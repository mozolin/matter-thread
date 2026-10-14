set(component_srcs "ssd1306_i2cdev.c" "ssd1306_spi.c" "ssd1306_i2cdev_new.c")

idf_component_register(
    SRCS "${component_srcs}"
    PRIV_REQUIRES driver
    INCLUDE_DIRS "."
    REQUIRES i2cdev
)
