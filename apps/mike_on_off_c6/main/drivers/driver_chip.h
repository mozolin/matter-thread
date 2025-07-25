#pragma once

//-- Getting chip temperature
extern float read_internal_temperature();

//-- Getting supply voltage
#include "esp_adc/adc_oneshot.h"

#if CONFIG_IDF_TARGET_ESP32C6
	#define ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED   1
	#define ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED    0
#endif

#if CONFIG_IDF_TARGET_ESP32H2
	#define ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED   0
	#define ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED    1
#endif

extern bool init_internal_voltage();
extern float read_internal_voltage();
extern void deinit_internal_voltage();
