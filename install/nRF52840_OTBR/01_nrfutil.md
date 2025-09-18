
# nRF Util and its packages

Если нет доступа к ресурсу домена *https://files.nordicsemi.com/* (403 Forbidden) и, значит, нет возможности использовать стандартные инструменты от Nordic Semiconductor (например, nRF Connect for Desktop Apps), можно воспользоваться [китайским "зеркалом"](https://files.nordicsemi.cn/ui/packages).  
[nRF Util branch](https://files.nordicsemi.cn/ui/repos/tree/General/swtools/external/nrfutil)  
  
![](images/nrfutil_cn.png)  


### Run nRF Util
~~~  
nrfutil
~~~
Возможная ошибка:  
  
> Error: Failed to bootstrap core functionality before executing command.  
> HTTP request to default bootstrap resource:  
> https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/index/bootstrap.json  
> failed with HTTP code: 403 Forbidden.  
> Please check that your internet connection is functioning. If you use a proxy, please try the --detect-proxy flag or manually set the appropriate HTTP_PROXY-style environment variable(s).  
> To use a custom bootstrap config, set NRFUTIL_BOOTSTRAP_CONFIG_URL.  
> To bootstrap directly from a nrfutil-core package tarball, set NRFUTIL_BOOTSTRAP_TARBALL_PATH.  
  
### Get original bootstrap.json
[Original bootstrap.json](https://files.nordicsemi.cn/ui/repos/tree/General/swtools-cache/external/nrfutil/index/bootstrap.json)  
~~~
{
  "nrfutil_core_tarball_urls": {
    "aarch64-apple-darwin": "https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/packages/nrfutil/nrfutil-aarch64-apple-darwin-8.0.0.tar.gz",
    "x86_64-apple-darwin": "https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/packages/nrfutil/nrfutil-x86_64-apple-darwin-8.0.0.tar.gz",
    "x86_64-pc-windows-msvc": "https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/packages/nrfutil/nrfutil-x86_64-pc-windows-msvc-8.0.0.tar.gz",
    "x86_64-unknown-linux-gnu": "https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/packages/nrfutil/nrfutil-x86_64-unknown-linux-gnu-8.0.0.tar.gz",
    "aarch64-unknown-linux-gnu": "https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/packages/nrfutil/nrfutil-aarch64-unknown-linux-gnu-8.0.0.tar.gz"
  }
}
~~~
Здесь могут возникнуть проблемы при скачивании tarball, если доступ к ресурсам *files.nordicsemi.com* закрыт.  

### Download nrfutil core tarball
[nrfutil-x86_64-pc-windows-msvc-8.1.1.tar.gz](https://files.nordicsemi.cn/ui/native/swtools-cache/external/nrfutil/packages/nrfutil/nrfutil-x86_64-pc-windows-msvc-8.1.1.tar.gz)
  
and put it to another web server (http://localhost, for instance):
~~~
{
  "nrfutil_core_tarball_urls": {
    "x86_64-pc-windows-msvc":    "https://mozolin.info/nrfutil/nrfutil-x86_64-pc-windows-msvc-8.1.1.tar.gz"
  }
}
~~~


### Make and run script
Run _run.cmd => files will be saved in C:\\Users\\[username]\\.nrfutil:  
~~~
@echo off
set NRFUTIL_BOOTSTRAP_CONFIG_URL=https://mozolin.info/nrfutil/bootstrap.json
nrfutil > nrfutil.txt
~~~
В случае удачного выполнение скрипта будет создана папка C:\\Users\\[username]\\.nrfutil
Check, if nRF Util istalled by getting its version:
~~~
nrfutil -V
~~~
> nrfutil 8.1.1 (b6089d0 2025-08-21)  
> commit-hash: b6089d08a9cfdb292f8ab8d21e0908ded814cd11  
> commit-date: 2025-08-21  
> host: x86_64-pc-windows-msvc  
> build-timestamp: 2025-08-21T14:12:43.593658000Z  
> classification: nrf-external  

### Download nRF Util packages
[nRF Util packages](https://files.nordicsemi.cn/ui/repos/tree/General/swtools/external/nrfutil/packages)


### Install nRF Util packages
~~~  
nrfutil install pkg
~~~
Возможная ошибка:  
  
> Error: Failed to query for package 'nrfutil-pkg' on package index - aborting before uninstall  
> Caused by:  
> HTTP request to 'https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/index/x86_64-pc-windows-msvc/nrfutil-pkg' was unsuccessful. Status code: **403 Forbidden**  

Возможная ошибка:  
~~~
nrfutil pkg
~~~
> Error: nrfutil command `pkg` not found. See `nrfutil list` for full list of installed commands, `nrfutil search` for installable commands, and `nrfutil install` for installation of new commands.  
> Caused by:  
> Subcommand nrfutil-pkg.exe not found  

1) Unpack "nrfutil-nrf5sdk-tools" package (nrfutil-nrf5sdk-tools-x86_64-pc-windows-msvc-1.1.0.tar.gz)
2) Put a copy of nrfutil-nrf5sdk-tools-x86_64-pc-windows-msvc-1.1.0 folder to C:\\Users\\[username]\\.nrfutil\\installed
3) Переместить содержимое папки nrfutil-nrf5sdk-tools-x86_64-pc-windows-msvc-1.1.0/data to C:\\Users\\[username]\\.nrfutil:
   - добавит содержимое папки bin в существующую папку bin
   - добавит папку lib
   - добавит содержимое папки share в существующую папку share
4) исправить пути к файлам в настройках C:\\Users\\[username]\\.nrfutil\\installed\\nrfutil-nrf5sdk-tools-x86_64-pc-windows-msvc-1.1.0\\files (заменить "data/" на "../../")
~~~
Filepath,Checksum
../../bin/nrfutil-dfu.exe,b0adce8dcadc7efeb5e413b84300dd37d2fcc526647529561c4317caf2fcb4dd
../../bin/nrfutil-keys.exe,3418fc195cebbe9e39a6664b5b1864f9326e322cf5dba9e78e502f4fc576d06c
../../bin/nrfutil-nrf5sdk-tools.exe,5dd5cb31ab9e65d17b59cc9f751d41f92f245de9472c16dc0fdda979610f782b
../../bin/nrfutil-pkg.exe,9c495bef81abd6aa03c0bf4f053a59ecba25d6c947e9b1ea55fc22365b07a140
../../bin/nrfutil-settings.exe,ff89e7933c4fc26273c3942423b4f9d326bb4739aa6f5b1f4456b670699ea873
../../bin/nrfutil-zigbee.exe,bf2defe93d41703108980f2fde12a2b837d93b22dfacecb493cc03bcacd7c356
../../lib/nrfutil-nrf5sdk-tools/pc_nrfutil_legacy_v6.1.7.exe,bac531e6781b9ab7a6bfccb519c8ee7a15b11ecf13140bb058cc7bb0cdc6d802
../../share/doc/nrfutil-nrf5sdk-tools/html/index.html,732c73d465122a04362635a884b79a44fe4645c0e35e6aa9da988efafc3a1a31
../../share/doc/nrfutil-nrf5sdk-tools/html/style.css,399b06d522d7ee9b8163e251697ce215fab063c242bbad17847130f9d991774f
../../share/doc/nrfutil-nrf5sdk-tools/md/index.md,497199adbd85f1778bde125370195dab9520d3bed24a55e422fb685c105124d3
../../share/nrfutil-nrf5sdk-tools/LICENSE,a2d699173aac0450b26b423acda8e589d5d4a1c3bd1d5653a3eb8bbd9b9f31b4
manifest.json,a7d898ded77381a448d8902b72c94071d191baf996762aeceba360763d55b01b
~~~
  
Проверить, что проблема устранена:
~~~
nrfutil pkg
~~~
>Usage: pc_nrfutil_legacy_v6.1.7.exe pkg [OPTIONS] COMMAND [ARGS]...  
>  This set of commands supports Nordic DFU package generation.  
>Options:  
>  --help  Show this message and exit.  
>Commands:  
>  display   Display the contents of a .zip package file.  
>  generate  Generate a zip file for performing DFU.  

Проделать всё то же самое для пакета "nrfutil-sdk-manager", также необходимого для создания и загрузки прошивки на nRF52840 USB Dongle.  

  
# Section Contents
- [Installing nRF Util and its packages](01_nrfutil.md)  
- [Creating RCP firmware for the nRF52840 USB Dongle](02_firmware.md)  
- [Installing and Configuring OTBR on Ubuntu](03_otbr.md)  
  