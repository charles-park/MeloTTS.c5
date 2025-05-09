# MeloTTS.c5
* MeloTTS library(Docker Image)를 이용한 기상안내 시스템
* ODROID-C5 + WeatherBoardZero + Audio amplifier board for ODROID-C5

### ODROID-C5 (2025-05-09)
* Linux OS Image : [factory-odroidc5-0307.img (odroidh server)](http://192.168.0.224:8080/S905X5M/ODROID-C5/Ubuntu/ubuntu-22.04-server-odroidc5-20250509.img.xz)
* WeatherBoardZero Wiki : https://wiki.odroid.com/accessory/sensor/weather_board_zero
* Audio amplifier board for ODROID-C5 Wiki : https://wiki.odroid.com/internal/accessory/add-on_board/audio_amplifier_board#software_setup

### MeloTTS
* Github : https://github.com/myshell-ai/MeloTTS/blob/main/docs/install.md#python-api
* Docker 파일 수정 및 추가사항
  - RUN pip install --upgrade pip
  - RUN pip install cached_path==1.1.3 botocore==1.29.76
  - CMD ["/bin/bash"] 
   
### Install package
```
// ubuntu package install
root@server:~# apt install build-essential vim ssh git python3 python3-pip ethtool net-tools usbutils i2c-tools overlayroot nmap evtest htop cups cups-bsd iperf3 alsa 

// ubuntu 24.01 version python3 package install
root@server:~# apt install python3-aiohttp python3-async-timeout

// docker install
root@server:~# apt install podman-docker

// system reboot
root@server:~# reboot

root@server:~# uname -a
Linux server 5.15.153-odroid-arm64 #1 SMP PREEMPT Tue, 22 Apr 2025 09:19:01 +0000 aarch64 aarch64 aarch64 GNU/Linux

```

### Github setting
```
root@server:~# git config --global user.email "charles.park@hardkernel.com"
root@server:~# git config --global user.name "charles-park"
```

### Clone the reopsitory with submodule
```
root@server:~# git clone -b client.v20 --recursive https://github.com/charles-park/JIG.Client

or

root@server:~# git clone -b client.v20 https://github.com/charles-park/JIG.Client
root@server:~# cd JIG.Client
root@server:~/JIG.Client# git submodule update --init --recursive
```

### Auto login
```
root@server:~# systemctl edit getty@tty1.service
```
```
[Service]
ExecStart=
ExecStart=-/sbin/agetty --noissue --autologin root %I $TERM
Type=idle
```
* edit tool save
  save exit [Ctrl+ k, Ctrl + q]


### Sound setup (TDM-C-T9015-audio-hifi-alsaPORT-i2s)
```
// Codec info
root@server:~# aplay -l
**** List of PLAYBACK Hardware Devices ****
card 0: AMLAUGESOUND [AML-AUGESOUND], device 0: TDM-B-dummy-alsaPORT-i2s2hdmi soc:dummy-0 []
  Subdevices: 1/1
  Subdevice #0: subdevice #0
card 0: AMLAUGESOUND [AML-AUGESOUND], device 1: SPDIF-B-dummy-alsaPORT-spdifb soc:dummy-1 []
  Subdevices: 1/1
  Subdevice #0: subdevice #0
card 0: AMLAUGESOUND [AML-AUGESOUND], device 2: TDM-C-T9015-audio-hifi-alsaPORT-i2s fe01a000.t9015-2 []
  Subdevices: 1/1
  Subdevice #0: subdevice #0
card 0: AMLAUGESOUND [AML-AUGESOUND], device 3: SPDIF-dummy-alsaPORT-spdif soc:dummy-3 []
  Subdevices: 1/1
  Subdevice #0: subdevice #0

// config mixer (mute off)
root@server:~# amixer sset 'TDMOUT_C Mute' off

// audio board setup
https://wiki.odroid.com/internal/accessory/add-on_board/audio_amplifier_board#software_setup

```

* Sound test (Sign-wave 1Khz)
```
// use speaker-test
root@server:~# speaker-test -D hw:0,2 -c 2 -t sine -f 1000           # pin header target, all
root@server:~# speaker-test -D hw:0,2 -c 2 -t sine -f 1000 -p 1 -s 1 # pin header target, left
root@server:~# speaker-test -D hw:0,2 -c 2 -t sine -f 1000 -p 1 -s 2 # pin header target, right

// or use aplay with (1Khz audio file)
root@server:~# aplay -D hw:0,2 {audio file} -d {play time}
root@server:~# aplay -D plughw:0,2 {audio file} -d {play time}
```

### Disable screen off
```
root@server:~# vi ~/.bashrc
...
setterm -blank 0 -powerdown 0 -powersave off 2>/dev/null
echo 0 > /sys/class/graphics/fb0/blank
...
```

### server static ip settings (For Debugging)
```
root@server:~# vi /etc/netplan/01-netcfg.yaml
```
```
network:
    version: 2
    renderer: networkd
    ethernets:
        eth0:
            dhcp4: no
            # static ip address
            addresses:
                - 192.168.20.162/24
            gateway4: 192.168.20.1
            nameservers:
              addresses: [8.8.8.8,168.126.63.1]

```
```
root@server:~# netplan apply
root@server:~# ifconfig
```

### Overlay root
* overlayroot enable
```
root@server:~# update-initramfs -c -k $(uname -r)
update-initramfs: Generating /boot/initrd.img-4.9.337-17

root@server:~# mkimage -A arm64 -O linux -T ramdisk -C none -a 0 -e 0 -n uInitrd -d /boot/initrd.img-$(uname -r) /boot/uInitrd 
Image Name:   uInitrd
Created:      Fri Oct 27 04:27:58 2023
Image Type:   AArch64 Linux RAMDisk Image (uncompressed)
Data Size:    7805996 Bytes = 7623.04 KiB = 7.44 MiB
Load Address: 00000000
Entry Point:  00000000

// Change overlayroot value "" to "tmpfs" for overlayroot enable
root@server:~# vi /etc/overlayroot.conf
...
overlayroot_cfgdisk="disabled"
overlayroot="tmpfs"
```
* overlayroot disable
```
// get write permission
root@server:~# overlayroot-chroot 
INFO: Chrooting into [/media/root-ro]
root@server:~# 

// Change overlayroot value "tmpfs" to "" for overlayroot disable
root@server:~# vi /etc/overlayroot.conf
...
overlayroot_cfgdisk="disabled"
overlayroot=""
```
