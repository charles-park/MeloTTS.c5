#!/bin/bash

sleep 1 && sync

# enable fb (VU5)
drm_setcrtc -d meson -m 800x480p60hz
# drm_setcrtc -d meson -m 1280x600p60hz
# drm_setcrtc -d meson -m 1920x1080p60hz
sleep 1
echo 0 | sudo tee /sys/devices/platform/drm-subsystem/graphics/fb0/blank
sleep 1

# enable audio (mute disable)
echo 488 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio488/direction
echo 1 > /sys/class/gpio/gpio488/value

amixer -c0 set 'TDMOUT_C Mute' off
sleep 1
