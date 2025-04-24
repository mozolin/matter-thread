# ESP Launchpad
ESP Launchpad helps to flash the selected firmware image onto your device.  
  
Here we can choose from some of ESP's pre-built, out-of-the-box examples to flash and play  
  
RainMaker examples: https://espressif.github.io/esp-launchpad/  
Matter examples: https://espressif.github.io/esp-launchpad/?flashConfigURL=https://espressif.github.io/esp-matter/launchpad.toml  
  
An external [source](https://espressif.github.io/esp-matter/launchpad.toml) can look like:  
~~~
esp_toml_version = 1.0
firmware_images_url = "https://espressif.github.io/esp-matter/"
config_readme_url = "https://espressif.github.io/esp-matter/build_cfg.md"

supported_apps = ["thread_matter_light","wifi_matter_light","wifi_matter_light_switch","wifi_rainmaker_matter_light"]

[thread_matter_light]
chipsets = ["ESP32C6","ESP32H2"]
image.esp32c6 = "esp32c6_thread_matter_light.bin"
image.esp32h2 = "esp32h2_thread_matter_light.bin"
ios_app_url = "https://apps.apple.com/app/esp-rainmaker/id1497491540"
android_app_url = "https://play.google.com/store/apps/details?id=com.espressif.rainmaker"
readme.text = "https://raw.githubusercontent.com/espressif/esp-matter/main/tools/launchpad/qrcode-content.md"

[wifi_matter_light]
chipsets = ["ESP32","ESP32C3","ESP32C6"]
image.esp32 = "esp32_wifi_matter_light.bin"
image.esp32c3 = "esp32c3_wifi_matter_light.bin"
image.esp32c6 = "esp32c6_wifi_matter_light.bin"
ios_app_url = "https://apps.apple.com/app/esp-rainmaker/id1497491540"
android_app_url = "https://play.google.com/store/apps/details?id=com.espressif.rainmaker"
readme.text = "https://raw.githubusercontent.com/espressif/esp-matter/main/tools/launchpad/qrcode-content.md"

[wifi_matter_light_switch]
chipsets = ["ESP32C3"]
image.esp32c3 = "esp32c3_wifi_matter_light_switch.bin"
ios_app_url = "https://apps.apple.com/app/esp-rainmaker/id1497491540"
android_app_url = "https://play.google.com/store/apps/details?id=com.espressif.rainmaker"
readme.text = "https://raw.githubusercontent.com/espressif/esp-matter/main/tools/launchpad/qrcode-content.md"

[wifi_rainmaker_matter_light]
chipsets = ["ESP32C3"]
image.esp32c3 = "esp32c3_wifi_rainmaker_matter_light.bin"
ios_app_url = "https://apps.apple.com/app/esp-rainmaker/id1497491540"
android_app_url = "https://play.google.com/store/apps/details?id=com.espressif.rainmaker"
readme.text = "https://raw.githubusercontent.com/espressif/esp-matter/main/tools/launchpad/qrcode-content.md"
~~~
See [source file](esp-launchpad/launchpad.toml)  
