# Office LAN with Internet Access
Example of creating a local network in an office.
  
![](office_network/OFFICE_NETWORK_x4500.jpg)  
  
### Features
- Internet access via a smartphone hotspot  
- Fixed IP addresses for all devices  
- Home Assistant on a dedicated Raspberry Pi microcomputer  
- Connecting Thread end devices via the Home Assistant app on a smartphone  
- OpenThread Border Router (Espressif)  
- OpenThread Border Router (SmLight SLZB-06)  
- OpenThread Border Router (nRF52840 USB Dongle)  
- Yandex Hub + Yandex Smart Home for connecting and displaying Matter devices  
- Dedicated Ubuntu computer for creating ESP-Matter firmware for Thread end devices  
- Office computer with a Wi-Fi connection to the local network  

### Hardware
| IP            | MAC               | Protocol         | Model                      | Vendor                |
|--------------:|:-----------------:|:----------------:|:---------------------------|:----------------------|
|   192.168.1.1 | DC:8E:8D:C5:64:64 | Wi-Fi / Ethernet | Netis N5 Router            | Netis                 |
| 192.168.1.100 | 00:E0:4C:4E:81:1F | Ethernet         | DEXP Mini Ethernet         | DEXP                  |
| 192.168.1.101 | DC:A6:32:1C:D1:2C | Ethernet         | HA Office Ethernet         | HA                    |
| 192.168.1.150 | 70:68:71:80:48:0C | Wi-Fi            | DEXP Mini Wi-Fi            | DEXP                  |
| 192.168.1.151 | DC:A6:32:1C:D1:2D | Wi-Fi            | HA Office Wi-Fi            | HA                    |
| 192.168.1.152 | 64:63:06:2E:CA:ED | Wi-Fi            | Redmi Note 14 Wi-Fi        | HA                    |
| 192.168.1.153 | 44:71:47:11:F3:AB | Wi-Fi            | Xiaomi 12S Ultra           | Xiaomi                |
| 192.168.1.201 | 1C:69:20:93:CA:2B | Ethernet         | SmLight SLZB-06 Ethernet   | SmLight               |
| 192.168.1.202 | 28:37:2F:E2:60:EB | Ethernet         | OTBR #4 Ethernet           | Espressif             |
| 192.168.1.203 | 28:37:2F:E2:AF:33 | Ethernet         | OTBR #5 Ethernet           | Espressif             |
| 192.168.1.204 | 3C:0B:4F:11:15:09 | Ethernet         | Yandex Hub Ethernet        | Yandex                |
| 192.168.1.234 | 14:DA:E9:B0:AE:18 | Wi-Fi            | HP Office PC               | HP                    |
| 192.168.1.250 | 7C:DF:A1:F3:56:58 | Wi-Fi            | OTBR #3 Wi-Fi              | Espressif             |
| 192.168.1.251 | 1C:69:20:93:CA:28 | Wi-Fi            | SmLight SLZB-06 Wi-Fi      | SmLight               |
| 192.168.1.252 | 28:37:2F:E2:60:E8 | Wi-Fi            | OTBR #4 Wi-Fi              | Espressif             |
| 192.168.1.253 | 28:37:2F:E2:AF:30 | Wi-Fi            | OTBR #5 Wi-Fi              | Espressif             |
  
- **Смартфон Redmi Note 14**  
  Подключён к ***мобильному интернету*** и создаёт Wi-Fi точку доступа.
- **Роутер Netis N5**  
  Подключён по ***Wi-Fi*** к точке доступа, созданной на смартфоне Redmi Note 14.  
  Использование роутера позволяет создавать независимую от точки доступа локальную сеть со своей собственной адресацией.
- **Коммутатор TP-Link LS1008**  
  Подключён через ***Ethernet*** к выходу роутера Netis N5.  
  Предназначен для расширения числа выходов Ethernet, поскольку роутер Netis N5 имеет только 2 таких выхода.
- **DEXP Mini**  
  Подключён через ***Ethernet*** к коммутатору TP-Link LS1008.  
  Работает под Ubuntu. Используется для компиляции C++ ESP-IDF/ESP-Matter приложений.
- **HA Office**  
  Подключён через ***Ethernet*** к коммутатору TP-Link LS1008.  
  Работает на Raspberry Pi 4 под HAOS. Используется для работы систем Home Assistant.
- **SmLight SLZB-06**  
  Подключён через ***Ethernet*** к коммутатору TP-Link LS1008.  
  Используется для создания OpenThread Boarder Router.
- **ESP OTBR #4**  
  Подключён через ***Ethernet*** к коммутатору TP-Link LS1008.  
  Используется для создания OpenThread Boarder Router.
- **ESP OTBR #5**  
  Подключён через ***Ethernet*** к коммутатору TP-Link LS1008.  
  Используется для создания OpenThread Boarder Router.
- **Yandex Hub**  
  Подключён через ***Ethernet*** к коммутатору TP-Link LS1008.  
  Используется для подключения Matter устройств.
- **ESP OTBR #3**  
  Подключён по ***Wi-Fi*** к коммутатору TP-Link LS1008.  
  Используется для создания OpenThread Boarder Router.
- **HP Office PC**  
  Подключён по ***Wi-Fi*** к коммутатору TP-Link LS1008.  
  Работает под Windows 10. Используется для более удобного редактирования исходного кода C++ приложений и скриптов Home Assistant. Папки с исходным кодом подключаются к системе Windows через службы Samba, установленные на компьютерах соответственно с ESP-IDF/ESP-Matter (под Ubuntu) и Home Assistant (под HAOS).
- **Xiaomi 12S Ultra**  
  Подключён по ***Wi-Fi*** к коммутатору TP-Link LS1008.  
  Используется для подключения конечных Matter устройств к Thread сети через мобильное приложение Home Assistant.


### Thread Network

- SmLight SLZB-06  
  Подключён через ***Ethernet*** к коммутатору TP-Link LS1008.  
  Используется для создания OpenThread Boarder Router.
- OTBR #3 Wi-Fi  
  Подключён по ***Wi-Fi*** к коммутатору TP-Link LS1008.  
  Используется для создания OpenThread Boarder Router.
- OTBR #4 Ethernet  
  Подключён через ***Ethernet*** к коммутатору TP-Link LS1008.  
  Используется для создания OpenThread Boarder Router.
- OTBR #5 Ethernet  
  Подключён через ***Ethernet*** к коммутатору TP-Link LS1008.  
  Используется для создания OpenThread Boarder Router.
- nRF52840 Dongle 1  
  Подключён через ***USB*** к компьютеру DEXP Mini (под Ubuntu).  
  Используется в качестве RPC-модуля для создания OpenThread Boarder Router под Ubuntu.
- nRF52840 Dongle 2  
  Подключён через ***USB*** к компьютеру DEXP Mini (под Ubuntu).  
  Используется в качестве RPC-модуля для создания OpenThread Boarder Router под Ubuntu.
- ESP32-H2 Mike Tiny RP4 RCP  
  Подключён через ***USB*** к компьютеру HA Office (на Raspberry Pi 4 под HAOS).  
  Используется в качестве RPC-модуля для создания OpenThread Boarder Router в дополнении "OpenThread Boarder Router" Home Assistant.
- ESP32-H2 Mike Tiny  
  В чип ESP32-H2 прошивается приложение "Mike Tiny" c минимальным функционалом для подключения к сети Thread.
- ESP32-H2 Mike OnOff  
  В чип ESP32-H2 прошивается приложение "Mike OnOff" с функционалом управления 8 реле для подключения к сети Thread.

### Настройка роутера Netis N5
- Главная страница "Advanced Setup"  
![](office_network/netis_01.png)  
  
- Подключение по Wi-Fi к точке доступа, созданной на смартфоне Redmi Note 14.  
![](office_network/netis_02.png)  

- Настройка локальной сети  
![](office_network/netis_03.png)  
  
- Привязка MAC к IP адресам для всех устройств в сети (резервирование адресов)  
![](office_network/netis_04.png)  

- Установка режима работы роутера как "Роутер"  
![](office_network/netis_05.png)  
  
- Использование IP4/IP6  
![](office_network/netis_06.png)  
  
- Настройка точки доступа на роутере для подключения беспроводных устройств на 2.4ГГц   
![](office_network/netis_07.png)  
  
- Настройка WPS на 2.4ГГц  
![](office_network/netis_08.png)  
  
- Отключение "AP Isolation" на 2.4ГГц  
![](office_network/netis_09.png)  

- Настройка точки доступа на роутере для подключения беспроводных устройств на 5ГГц  
![](office_network/netis_10.png)  

- Настройка WPS на 5ГГц  
![](office_network/netis_11.png)  

- Отключение "AP Isolation" на 5ГГц  
![](office_network/netis_12.png)  
  
