#!/usr/bin/env bash

echo 'CONFIG_OPENTHREAD_NETWORK_AUTO_START=y' > sdkconfig.thread
SDKCONFIG_DEFAULTS='sdkconfig.defaults;sdkconfig.thread' idf.py build

idf.py merge-bin
