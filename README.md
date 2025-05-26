# MeloTTS.c5
* MeloTTS library(Docker Image)를 이용한 기상안내 시스템
* ODROID-C5 + WeatherBoardZero + Audio amplifier board for ODROID-C5

### ODROID-C5 (2025-05-09)
* Linux OS Image : [factory-odroidc5-0307.img (odroidh server)](http://192.168.0.224:8080/S905X5M/ODROID-C5/Ubuntu/ubuntu-22.04-server-odroidc5-20250509.img.xz)
* WeatherBoardZero Wiki : https://wiki.odroid.com/accessory/sensor/weather_board_zero
* Audio amplifier board for ODROID-C5 Wiki : https://wiki.odroid.com/internal/accessory/add-on_board/audio_amplifier_board#software_setup

### Install package
```
// ubuntu package install
root@server:~# apt install build-essential vim ssh git python3 python3-pip ethtool net-tools usbutils i2c-tools overlayroot nmap evtest htop cups cups-bsd iperf3 alsa libcurl4-openssl-dev libcjson-dev

// ubuntu 24.01 version python3 package install
root@server:~# apt install python3-aiohttp python3-async-timeout

// *** Docker Install ***
// 필수 패키지 설치
root@server:~# apt install -y \
    ca-certificates \
    curl \
    gnupg \
    lsb-release

// Docker 공식 GPG 키 추가
root@server:~# mkdir -p /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | \
    gpg --dearmor -o /etc/apt/keyrings/docker.gpg

// Docker 리포지터리 추가 (arm64 지원 포함)
root@server:~# echo \
  "deb [arch=arm64 signed-by=/etc/apt/keyrings/docker.gpg] \
  https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | \
  tee /etc/apt/sources.list.d/docker.list > /dev/null

// Docker 엔진설치
root@server:~# apt update && apt install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

// Docker network(iptable) 관련설정이 없거나 지원하지 않는 경우(ODROID-C5는 지원하지 않음)
// 이 경우 docker실행시 --network=host명령을 주어 host의 network을 사용하도록 함.
root@server:~# mkdir -p /etc/docker
root@server:~# tee /etc/docker/daemon.json > /dev/null <<EOF
{
  "iptables": false
}
EOF

// system reboot
root@server:~# reboot

root@server:~# uname -a
Linux server 5.15.153-odroid-arm64 #1 SMP PREEMPT Tue, 22 Apr 2025 09:19:01 +0000 aarch64 aarch64 aarch64 GNU/Linux

```

### MeloTTS
* Github : https://github.com/myshell-ai/MeloTTS (https://github.com/myshell-ai/MeloTTS/blob/main/docs/install.md)
* Docker 파일 수정 및 추가사항
  - RUN pip install --upgrade pip
  - RUN pip install cached_path==1.1.3 botocore==1.29.76
  - CMD ["/bin/bash"]
* Docker Build (kernel network package가 정상적으로 설치되지 않은 경우)
  - docker build --network=host -t melotts .

* Docker 실행 (실행폴더를 공유함)
  - docker run --rm --network=host -it -v $(pwd):/app melotts # 컨테이너 종료시 삭제

* Docker Image/Container 삭제
  - docker rm -f $(docker ps -aq)           # 모든 컨테이너 삭제
  - docker rmi -f $(docker images -q)       # 모든 이미지 삭제
  - docker volume prune -f 

### Github setting
```
root@server:~# git config --global user.email "charles.park@hardkernel.com"
root@server:~# git config --global user.name "charles-park"
```

### Clone the reopsitory with submodule
```
root@server:~# git clone --recursive https://github.com/charles-park/melotts.c5

or

root@server:~# git clone https://github.com/charles-park/melotts.c5
root@server:~# cd melotts.c5
root@server:~/melotts.c5# git submodule update --init --recursive
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

// audio board setup wiki : https://wiki.odroid.com/internal/accessory/add-on_board/audio_amplifier_board#software_setup
// h/w mute disable
root@server:~# echo 488 > /sys/class/gpio/export
root@server:~# echo out > /sys/class/gpio/gpio488/direction
root@server:~# echo 1 > /sys/class/gpio/gpio488/value

// config mixer (mute off)
root@server:~# amixer -c0 set 'TDMOUT_C Mute' off
Simple mixer control 'TDMOUT_C Mute',0
  Capabilities: pswitch pswitch-joined
  Playback channels: Mono
  Mono: Playback [off]

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
