# Props https://github.com/sudar/Arduino-Makefile

BOARD_TAG    = mega2560
ARDUINO_LIBS =

include Arduino-Makefile/Arduino.mk

monitor:
	minicom -D $(call get_monitor_port) -b $(MONITOR_BAUDRATE)
