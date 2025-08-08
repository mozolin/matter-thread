idf_component_register(
	SRC_DIRS "." "./drivers"
	SRC_DIRS "." "./plugins"
  PRIV_INCLUDE_DIRS "." "./drivers" "./plugins" "${ESP_MATTER_PATH}/examples/common/utils"
)

set(ZAP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/main/zap/custom.zap")

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
#target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
