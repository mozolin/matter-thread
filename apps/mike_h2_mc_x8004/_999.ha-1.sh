#!/usr/bin/env bash

#sudo apt install -y socat

# Трафик виртуальной ноды пересылаем на физический OTBR
socat UDP4-LISTEN:9002,fork UDP4:192.168.31.104:9001 &

esp-emu \
  --chip esp32h2 \
  --firmware build/merged-binary.bin \
  --elf build/mike_h2_multiclimate.elf \
  --net user \
  --thread-sim bind:9001,peer:127.0.0.1:9002 \
  --inject-on "Server Listening..." \
  --inject "matter esp ot_cli dataset set active 0e080000000000010000000300000f4a0300001035060004001fffe00208dead00beef00cafe0708fd000db800a00000051000112233445566778899aabbccddeeff030e4f70656e5468726561642d455350010212340410104810e2315100afd6bc9215a6bfac530c0402a0f7f8\n"
