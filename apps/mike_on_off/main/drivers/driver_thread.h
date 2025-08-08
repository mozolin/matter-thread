#pragma once

#include <app_priv.h>

#if USE_OPENTHREAD_DRIVER

extern void ot_init_thread_time_sync();
void ot_on_thread_time_updated(void* context);
extern char* ot_get_thread_uptime(char* str);
extern char* ot_get_thread_short_uptime(char* str);
extern bool ot_get_current_thread_time(struct tm* timeinfo);

#endif
