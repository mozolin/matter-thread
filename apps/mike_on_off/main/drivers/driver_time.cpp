
#include "driver_time.h"
#include "esp_sntp.h"
#include "esp_netif_sntp.h"
#include <time.h>

#include <system/SystemClock.h>
#include <app/server/Server.h>

#if USE_TIME_DRIVER
  void sntp_initialize_time()
  {
    ESP_LOGW(TAG_MIKE_APP, "~~~ Initializing SNTP");

    esp_sntp_stop();
    esp_netif_sntp_deinit();

    //esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_setservername(2, "time.nist.gov");
    esp_sntp_setservername(3, "0.pool.ntp.org");
		esp_sntp_setservername(4, "1.pool.ntp.org");
		esp_sntp_setservername(5, "2.pool.ntp.org");
    esp_sntp_init();
    
    if(esp_sntp_enabled()) {
    	ESP_LOGW(TAG_MIKE_APP, "~~~ esp_sntp_enabled() == true");
    }
    uint8_t num = esp_sntp_getreachability(0);
    ESP_LOGW(TAG_MIKE_APP, "~~~ esp_sntp_getreachability(0) == %d", num);

    //-- Waiting for synchronization (max. 10 seconds)
    int retry = 0;
    const int retry_count = 10;
    while(sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
      ESP_LOGW(TAG_MIKE_APP, "~~~ Waiting for system time to be set... (%d/%d)", retry, retry_count);
      vTaskDelay(pdMS_TO_TICKS(2000));
    }

    if(!sntp_is_synced()) {
    	ESP_LOGE(TAG_MIKE_APP, "~~~ SNTP is not synchronized!");
    	return;
    }

    ESP_LOGW(TAG_MIKE_APP, "~~~ SNTP: SUCCESS!");
    //-- Setting the time zone: Moscow time (UTC+3)
    setenv("TZ", "MSK-3", 1);
    tzset();
  }

  /*
  #include <esp_netif.h>
	#include <esp_openthread.h>
	#include <esp_openthread_netif_glue.h>

	static esp_netif_t* openthread_netif = NULL;
	*/
	
  void netif_sntp_initialize_time()
  {
    ESP_LOGW(TAG_MIKE_APP, "~~~ Initializing NETIF SNTP");

    /*
    // 1. Create a network interface configuration
    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_DEF_OT();
    
    // 2. Create a config for esp_netif_new
    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_OPENTHREAD();
    netif_config.base = &esp_netif_config;
    
    // 3. Create an interface
    openthread_netif = esp_netif_new(&netif_config);
    
    // 4. Initialize OpenThread
    esp_openthread_platform_config_t ot_config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()
    };
    if(esp_openthread_init(&ot_config) != ESP_OK) {
    	ESP_LOGE(TAG_MIKE_APP, "~~~ esp_openthread_init() failed!");
    }
    
    // 5. Bind the network interface
    esp_openthread_netif_glue_init(&ot_config);
    */

    // 6. Stop SNTP if it was running
    esp_netif_sntp_deinit();
    
    // 7. Setting up parameters
    //esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("time.google.com");
    config.start = true; // Autostart
    config.server_from_dhcp = false;
    config.renew_servers_after_new_IP = true;
    
    // 8. Initialize
    if(esp_netif_sntp_init(&config) != ESP_OK) {
    	ESP_LOGE(TAG_MIKE_APP, "~~~ esp_netif_sntp_init() failed!");
    	return;
    }

    // 9. Wait 10 seconds for synchronization
    if(esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
    	ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to update system time within 10s timeout");
    	return;
		}

    ESP_LOGW(TAG_MIKE_APP, "~~~ Netif SNTP: SUCCESS!");
    //-- 10. Setting the time zone: Moscow time (UTC+3)
    setenv("TZ", "MSK-3", 1);
    tzset();
  }

  bool sntp_is_synced()
  {
    return sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED;
	}


	bool chip_get_current_matter_time(struct tm *timeinfo)
	{
    // 1. Пытаемся получить Matter-время
    chip::System::Clock::Microseconds64 matterTime;
    if(chip::System::SystemClock().GetClock_RealTime(matterTime) == CHIP_NO_ERROR) {
      time_t time = static_cast<time_t>(matterTime.count() / 1000000);
      if(time < 1600000000) { // Фильтр дат до 2020 года
        localtime_r(&time, timeinfo);
        return false;
      }
    }

    // 2. Fallback на системное время
    time_t now = time(nullptr);
    if(now < 1600000000) {
      localtime_r(&now, timeinfo);
      return false;
    }

    //return false;
    
    
    chip::System::Clock::Microseconds64 utcTime;
    if(chip::System::SystemClock().GetClock_RealTime(utcTime) != CHIP_NO_ERROR) {
      return false;
    }

    // Проверка на "эпоху по умолчанию" (1 Jan 2000)
    if(utcTime.count() <= 946'684'800'000'000) { // 2000-01-01 в микросекундах
      return false;
    }

    time_t epoch = static_cast<time_t>(utcTime.count() / 1'000'000);
    if(epoch > 1600000000) { // Фильтр дат до 2020 года
    	localtime_r(&epoch, timeinfo);
    	return true;
    }
    
    // Используем тип Microseconds64
    //chip::System::Clock::Microseconds64 utcTime;
    
    // Получаем время через SystemClock
    if(chip::System::SystemClock().GetClock_RealTime(utcTime) != CHIP_NO_ERROR) {
      return false;
    }
    
    // Конвертируем в time_t
    time_t time = static_cast<time_t>(utcTime.count() / 1000000); // мкс → сек
    localtime_r(&time, timeinfo);
    
    ESP_LOGW(TAG_MIKE_APP, "~~~ Matter Time: SUCCESS!");
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
    ESP_LOGW(TAG_MIKE_APP, "~~~ Time Now - 1: %d", (int)now);
    chip::System::Clock::Microseconds64 chipTime = 
        chip::System::Clock::Microseconds64(static_cast<uint64_t>(now) * 1000000);
    
    chip::System::SystemClock().SetClock_RealTime(chipTime);
	}

	/*
	~~~~~~~~~~~~~
	 System Time
	~~~~~~~~~~~~~
	https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/system_time.html
	*/
	#include "esp_netif_sntp.h"
	void show_time()
	{
		time_t now;
		char strftime_buf[64];
		struct tm timeinfo;
		time(&now);
		setenv("TZ", "MSK-3", 1);
		tzset();
		localtime_r(&now, &timeinfo);
		strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
		ESP_LOGW(TAG_MIKE_APP, "~~~ The current date/time in St.Petersburg, Russia is: %s", strftime_buf);
		//vTaskDelay(pdMS_TO_TICKS(10000));
	  
	  /*
	  //-- 
	  sp_netif_sntp_deinit();
		
		esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
		config.start = true; // Autostart
    config.server_from_dhcp = false;
    config.renew_servers_after_new_IP = true;

		esp_netif_sntp_init(&config);
		if(esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
      ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to update system time within 10s timeout!");
      return;
		}
		//vTaskDelay(pdMS_TO_TICKS(10000));
		ESP_LOGW(TAG_MIKE_APP, "~~~ Show Time: SUCCESS!");
		*/
	}
#endif
