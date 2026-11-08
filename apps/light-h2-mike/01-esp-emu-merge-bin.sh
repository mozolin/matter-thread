#!/usr/bin/env bash

idf.py set-target esp32c3
idf.py build

idf.py merge-bin
