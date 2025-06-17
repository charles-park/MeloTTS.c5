#!/bin/bash
systemctl disable odroid-c5_hw.service && sync

cp ./odroid-c5_hw.service /etc/systemd/system/ && sync

systemctl enable odroid-c5_hw.service && sync

systemctl restart odroid-c5_hw.service && sync

