# --- nano ide 1.6
#ARDUINO_LIBS = Wire LiquidCrystal_I2C Rotary
BOARD_TAG    = nano
BOARD_SUB    = atmega328
ARDUINO_DIR  = /home/mitch/Downloads/arduino-1.8.10
ARDMK_VENDOR = arduino
MONITOR_PORT = /dev/ttyUSB*
ADRMK_DIR    = /home/mitch/builds/Arduino-Makefile
include /home/mitch/builds/Arduino-Makefile/Arduino.mk
