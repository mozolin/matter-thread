
# Подготовка
~~~
# Создаем TAP-интерфейс
sudo ip tuntap add dev tap0 mode tap user mike
sudo ip link set tap0 up
sudo chmod 0666 /dev/net/tun

# Установлен ли Bluetooth?
systemctl status bluetooth

# Перевод hci0 в режим Raw/Down
sudo hciconfig hci0 down
# Находим, где именно установлен esp-emu
ESP_EMU_PATH=$(which esp-emu)
# Выдаем права на управление сетью и raw-сокетами
sudo setcap 'cap_net_admin,cap_net_raw+ep' $ESP_EMU_PATH

# Добавляем пользователя в группу bluetooth
sudo usermod -aG bluetooth $USER
su - $USER
# проверяем
groups
~~~

# Создание прошивки
~~~
#!/usr/bin/env bash

idf.py merge-bin
~~~

# Запуск прошивки в режиме эмулятора
~~~
#!/usr/bin/env bash

esp-emu --chip esp32h2 \
        --firmware build/merged-binary.bin \
        --elf build/mike_h2_multiclimate.elf \
        --net tap,ifname=tap0 \
        --ble-hci hci0
~~~
