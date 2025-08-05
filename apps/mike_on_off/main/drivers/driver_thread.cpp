
#include "driver_thread.h"

#if USE_THREAD_DRIVER

	#include <time.h>
	#include <openthread/instance.h>
	#include <openthread/network_time.h>
	#include <openthread/platform/time.h>

	#include <cmath>

	static otInstance* sThreadInstance = NULL;

	// Инициализация Thread
	void ot_init_thread_time_sync()
	{
    // 1. Настройка времени
    sThreadInstance = otInstanceInitSingle();
    if(!otInstanceIsInitialized(sThreadInstance)) {
    	ESP_LOGE(TAG_MIKE_APP, "~~~ OpenThread Instance is not initialized!");
    	return;
    }
    otNetworkTimeSyncSetCallback(sThreadInstance, &ot_on_thread_time_updated, NULL);
    
    ESP_LOGW(TAG_MIKE_APP, "~~~ OpenThread Instance is initialized!");
    
    // 2. Установка часового пояса
    setenv("TZ", "MSK-3", 1); // Для Москвы (UTC+3)
    tzset();
	}

	// Callback при обновлении времени
	void ot_on_thread_time_updated(void* context)
	{
    otNetworkTimeStatus networkTimeStatus;
    uint64_t threadTime;
    
    networkTimeStatus = otNetworkTimeGet(sThreadInstance, &threadTime);
    ESP_LOGW(TAG_MIKE_APP, "~~~ OpenThread Time updated with status %d", networkTimeStatus);
    
    if(networkTimeStatus == OT_NETWORK_TIME_SYNCHRONIZED) {
      ESP_LOGW(TAG_MIKE_APP, "~~~ Synced time: %llu", threadTime);
    }
	}

	char* ot_get_thread_uptime(char* str)
	{
		size_t max_size = OT_UPTIME_STRING_SIZE;
		char buf[max_size];
		uint64_t uptime = 0;

		if(!otInstanceIsInitialized(sThreadInstance)) {
    	//-- returns an empty string
    	strncpy(str, "", max_size);
    	return str;
    }

    //-- as an interger value
    uptime = otInstanceGetUptime(sThreadInstance);

    //-- as a string value
    otInstanceGetUptimeAsString(sThreadInstance, buf, sizeof(buf));

    ESP_LOGW(TAG_MIKE_APP, "~~~ OpenThread Uptime: %llu (%s)", uptime, buf);
    strncpy(str, buf, max_size);

    return str;
	}

	char* ot_get_thread_short_uptime(char* str)
	{
		size_t max_size = OT_UPTIME_STRING_SIZE;
		char buf[max_size];
		uint64_t uptime = 0;

		if(!otInstanceIsInitialized(sThreadInstance)) {
    	//-- returns an empty string
    	strncpy(str, "", max_size);
    	return str;
    }

    //-- as an interger value
    uptime = otInstanceGetUptime(sThreadInstance);

    //-- parse uptime value
    int total = (uptime - (uptime % 1000)) / 1000;
    int hhh = std::floor(total / 3600);
		int mmm = std::floor((total / 60) % 60);
		int sss = total % 60;

		snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hhh, mmm, sss);

    ESP_LOGW(TAG_MIKE_APP, "~~~ OpenThread Uptime: %llu (%s)", uptime, buf);
    strncpy(str, buf, max_size);

    return str;
	}
	
	bool ot_get_current_thread_time(struct tm* timeinfo)
	{
    if(!otInstanceIsInitialized(sThreadInstance)) {
    	return false;
    }
    ESP_LOGW(TAG_MIKE_APP, "~~~ OpenThread Instance is initialized!");

    uint64_t timeSinceEpoch = otPlatTimeGet();
    // Конвертация в Unix time (секунды с 1970)
    time_t unixTime = static_cast<time_t>(timeSinceEpoch / 1000000); // мкс → с
    localtime_r(&unixTime, timeinfo);
    ESP_LOGW(TAG_MIKE_APP, "~~~ otPlatTimeGet(): %d", (int)unixTime);
    
    // Получаем время через новый API
    otNetworkTimeStatus networkTimeStatus;
    uint64_t threadTime;
    
    networkTimeStatus = otNetworkTimeGet(sThreadInstance, &threadTime);
    ESP_LOGW(TAG_MIKE_APP, "~~~ OpenThread Time status: %d (should be %d)", networkTimeStatus, OT_NETWORK_TIME_SYNCHRONIZED);
    
    if(networkTimeStatus != OT_NETWORK_TIME_SYNCHRONIZED) {
      return false;
    }
    
    // Конвертируем Thread время (секунды с 1970-01-01)
    unixTime = static_cast<time_t>(threadTime);
    localtime_r(&unixTime, timeinfo);
    ESP_LOGW(TAG_MIKE_APP, "~~~ OpenThread Time: %d", (int)unixTime);
    
    return true;
	}

#endif
