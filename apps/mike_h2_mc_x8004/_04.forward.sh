#!/usr/bin/env bash

# Создаем TAP-интерфейс 0
sudo ip tuntap add dev tap0 mode tap
# Включаем IPv6-маршрутизацию на интерфейсе
sudo ip link set tap0 up
sudo sysctl -w net.ipv6.conf.tap0.forwarding=1

# Создаем TAP-интерфейс 1
sudo ip tuntap add dev tap1 mode tap
# Включаем IPv6-маршрутизацию на интерфейсе
sudo ip link set tap1 up
sudo sysctl -w net.ipv6.conf.tap1.forwarding=1
