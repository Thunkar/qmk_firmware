VIA_ENABLE = yes
VIAL_ENABLE = yes
OLED_ENABLE = yes
ENCODER_ENABLE = yes
POINTING_DEVICE_DRIVER = custom
ANALOG_DRIVER_REQUIRED = yes
CONSOLE_ENABLE = yes

# Add library files for modular code organization
SRC += ../../lib/adc.c \
       ../../lib/encoder.c \
       ../../lib/pointing_device.c \
       ../../lib/split_sync.c

# Add lib directory to include path
VPATH += ../../lib
EXTRAINCDIRS += ../../lib