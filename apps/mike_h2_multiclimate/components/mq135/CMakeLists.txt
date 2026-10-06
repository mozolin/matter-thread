set(component_srcs "mq135_driver.c")

idf_component_register(
    SRCS "${component_srcs}"
    PRIV_REQUIRES driver
    INCLUDE_DIRS "."
)
