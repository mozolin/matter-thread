#pragma once

#include <app_priv.h>

#if USE_TIME_DRIVER

  extern void sntp_initialize_time(void);
  extern bool sntp_is_synced();

	extern bool chip_get_current_matter_time(struct tm *timeinfo);
	extern bool chip_is_time_synchronized();
	extern void chip_initialize_time();

#endif
