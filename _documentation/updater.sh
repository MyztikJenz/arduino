#!/bin/sh -x
PKG_ROOT="/Users/jimt/Documents/Arduino/_documentation/ESP8266_NONOS_SDK-2.2.1"
PORT="/dev/cu.usbserial-12345678"
esptool.py --port $PORT write_flash \
0x00000 $PKG_ROOT/bin/boot_v1.7.bin \
0x01000 $PKG_ROOT/bin/at/512+512/user1.1024.new.2.bin \
0xfc000 $PKG_ROOT/bin/esp_init_data_default_v08.bin \
0x7e000 $PKG_ROOT/bin/blank.bin \
0xfe000 $PKG_ROOT/bin/blank.bin

# Don't use this... was trying to write a 2MB image to a 1MB ESP8266. Apparently all the esp-01's I bought are esp-01s. ¯\_(ツ)_/¯
# esptool.py --port $PORT write_flash \
# 0x00000 $PKG_ROOT/bin/boot_v1.7.bin \
# 0x01000 $PKG_ROOT/bin/at/1024+1024/user1.2048.new.5.bin \
# 0x1fc000 $PKG_ROOT/bin/esp_init_data_default_v08.bin \
# 0xfe000 $PKG_ROOT/bin/blank.bin \
# 0x1fe000 $PKG_ROOT/bin/blank.bin
