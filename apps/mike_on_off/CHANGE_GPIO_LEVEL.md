
# Change GPIO level for relays

~~~
app_driver.cpp:

app_driver_plugin_unit_init(const gpio_plug* plug)
{
  gpio_set_level(plug->GPIO_PIN_VALUE, 0);
}

app_driver_update_gpio_value(gpio_num_t pin, bool value)
{
  gpio_set_level(pin, value);
}


app_driver_attribute_update(...)
{
	app_driver_update_gpio_value(gpio_pin, val->val.b);
}
~~~


# Set reset button using BSP (Board Support Package)
~~~
sdkconfig (ESP32-H2 - has 2 buttons on board):

CONFIG_BSP_BUTTONS_NUM=2
#
# Button 1
#
CONFIG_BSP_BUTTON_1_TYPE_GPIO=y
CONFIG_BSP_BUTTON_1_GPIO=0
CONFIG_BSP_BUTTON_1_LEVEL=0
~~~
