#pragma once

#if USE_INTERNAL_TEMPERATURE
	//-- Getting chip temperature
	extern float read_internal_temperature();
#endif


//-- Getting supply voltage0
#if USE_INTERNAL_VOLTAGE
  #include "esp_adc/adc_oneshot.h"

  extern bool init_internal_voltage();
  extern float read_internal_voltage();
  extern void deinit_internal_voltage();
#endif
