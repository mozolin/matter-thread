idf_component_register(
	SRC_DIRS "." "./drivers" "./plugins"
  PRIV_INCLUDE_DIRS "." "./drivers" "./plugins" "${ESP_MATTER_PATH}/examples/common/utils"
)

# Добавляем флаги обертки (wrapping) для линкера GCC
target_link_options(${COMPONENT_LIB} PRIVATE 
    "-Wl,--wrap=esp_bt_controller_init"
    "-Wl,--wrap=esp_bt_controller_enable"
    "-Wl,--wrap=esp_nimble_init"
    "-Wl,--wrap=nimble_port_init"
    "-Wl,--wrap=nimble_port_run"
    "-Wl,--wrap=ble_gap_adv_start"
    "-Wl,--wrap=ble_gap_adv_stop"
    "-Wl,--wrap=ble_gap_adv_set_fields"
    "-Wl,--wrap=ble_gap_update_params"
    "-Wl,--wrap=ble_gap_terminate"
)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
