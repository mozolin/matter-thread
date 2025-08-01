
#include "driver_time.h"
#include "esp_sntp.h"
#include <time.h>

#include <system/SystemClock.h>
#include <app/server/Server.h>

#if USE_TIME_DRIVER
  void sntp_initialize_time()
  {
    ESP_LOGW(TAG_MIKE_APP, "Initializing SNTP");

    esp_sntp_stop();
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_setservername(2, "time.nist.gov");
    esp_sntp_setservername(3, "0.pool.ntp.org");
		esp_sntp_setservername(4, "1.pool.ntp.org");
		esp_sntp_setservername(5, "2.pool.ntp.org");
    esp_sntp_init();
    
    //-- Waiting for synchronization (max. 10 seconds)
    int retry = 0;
    const int retry_count = 10;
    while(sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
      ESP_LOGW(TAG_MIKE_APP, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }

    //-- Setting the time zone: Moscow time (UTC+3)
    setenv("TZ", "MSK-3", 1);
    tzset();
  }

  bool sntp_is_synced()
  {
    return sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED;
	}



	bool chip_get_current_matter_time(struct tm *timeinfo)
	{
    // Используем новый тип Microseconds64
    chip::System::Clock::Microseconds64 utcTime;
    
    // Получаем время через SystemClock
    if (chip::System::SystemClock().GetClock_RealTime(utcTime) != CHIP_NO_ERROR) {
        return false;
    }
    
    // Конвертируем в time_t
    time_t time = static_cast<time_t>(utcTime.count() / 1000000); // мкс → сек
    localtime_r(&time, timeinfo);
    
    return true;
	}

	bool chip_is_time_synchronized()
	{
    chip::System::Clock::Microseconds64 utcTime;
    return chip::System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR;
	}

	void chip_initialize_time()
	{
    // Установка текущего системного времени
    time_t now = time(NULL);
    chip::System::Clock::Microseconds64 chipTime = 
        chip::System::Clock::Microseconds64(static_cast<uint64_t>(now) * 1000000);
    
    chip::System::SystemClock().SetClock_RealTime(chipTime);
	}
#endif
