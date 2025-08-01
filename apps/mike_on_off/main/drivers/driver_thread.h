#pragma once

#include <app_priv.h>

#if USE_THREAD_DRIVER

extern void ot_init_thread_time_sync();
void ot_on_thread_time_updated(void* context);
extern bool ot_get_current_thread_time(struct tm* timeinfo);

#endif
