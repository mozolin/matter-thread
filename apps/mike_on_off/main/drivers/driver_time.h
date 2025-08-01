#pragma once

#include <app_priv.h>

#if USE_TIME_DRIVER
	
	#define ESP_NETIF_DEF_OT() \
  {   \
    .flags = ESP_NETIF_FLAG_AUTOUP, \
    ESP_COMPILER_DESIGNATED_INIT_AGGREGATE_TYPE_EMPTY(mac) \
    ESP_COMPILER_DESIGNATED_INIT_AGGREGATE_TYPE_EMPTY(ip_info) \
    .get_ip_event = 0,    \
    .lost_ip_event = 0,   \
    .if_key = "OT_DEF",  \
    .if_desc = "openthread",    \
    .route_prio = 15      \
	};

  extern void sntp_initialize_time(void);
  extern void netif_sntp_initialize_time(void);
  extern bool sntp_is_synced();

	extern bool chip_get_current_matter_time(struct tm *timeinfo);
	extern bool chip_is_time_synchronized();
	extern void chip_initialize_time();

	extern void show_time(void);

#endif
