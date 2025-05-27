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
root@server:~# apt install build-essential vim ssh git python3 python3-pip ethtool net-tools usbutils i2c-tools overlayroot nmap evtest htop cups cups-bsd iperf3 alsa libcurl4-openssl-dev libcjson-dev tree

// ubuntu 24.01 version python3 package install
root@server:~# pip install nltk

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


### MeloTTS
* Github : https://github.com/myshell-ai/MeloTTS (https://github.com/myshell-ai/MeloTTS/blob/main/docs/install.md)
    - git clone https://github.com/myshell-ai/MeloTTS MeloTTS.lib

* Docker 파일 수정 및 추가사항 (library.update folder안에 있는 모든 파일을 MeloTTS lib폴더안에 복사한다.)
  - RUN pip install --upgrade pip
  - RUN pip install cached_pa#
  - RUN pip install nltk
  - COPY nltk_data /app
  - ENV NLTK_DATA=/app/nltk_datath==1.34.88 botocore==1.6.2
  - ENTRYPOINT ["python", "./mk_speech.py"]

* RUN python melo/init_downloads.py를 주석처리(리소스부족의 문제로 nlk_data는 먼저 다운로드 하도록한다.)
```
#
# ARM64에서 Docker image 작업중 nlk_data 다운로드시 리소스 문제로 정상적으로 동작하지 않는 경우가 생김
#
# RUN python melo/init_downloads.py
#
# init_downloads문제가 발생하여 수동으로 먼저 다운로드 후 PATH를 설정하여 사용하도록 수정함.
#

# nltk_data는 docker root 즉 Docker file이 있는 폴더 아래에 반드시 존재하여야 함.
# 따라서 melo clone한 디렉토리 안에서 아래와 같이 실행하여 다운로드 함
#
# apt install python3 python3-pip
# pip install nltk
# mkdir -p ./nltk_data
# python3 -m nltk.downloader -d ./nltk_data cmudict

root@server:~/melotts.c5/weather.app/MeloTTS.lib# pip install nltk
root@server:~/melotts.c5/weather.app/MeloTTS.lib# mkdir -p ./nltk_data
root@server:~/melotts.c5/weather.app/MeloTTS.lib# python3 -m nltk.downloader -d ./nltk_data cmudict
```

* Docker Build (kernel network package가 정상적으로 설치되지 않은 경우)
  - docker build --no-cache --network=host -t melotts .

```
RUN python melo/init_downloads.pydocker build --no-cache --network=host -t melotts .

[+] Building 777.0s (15/15) FINISHED                             docker:default
 => [internal] load build definition from Dockerfile                       0.0s
 => => transferring dockerfile: 1.14kB                                     0.0s
 => [internal] load metadata for docker.io/library/python:3.9-slim         1.5s
 => [internal] load .dockerignore                                          0.0s
 => => transferring context: 2B                                            0.0s
 => [internal] load build context                                          0.8s
 => => transferring context: 33.88MB                                       0.7s
 => [ 1/10] FROM docker.io/library/python:3.9-slim@sha256:aff2066ec8914f7  5.3s
 => => resolve docker.io/library/python:3.9-slim@sha256:aff2066ec8914f738  0.0s
 => => sha256:b16f1b16678093d11ecfece1004207a40f9bc1b7d 28.07MB / 28.07MB  0.7s
 => => sha256:8a45c7e905d6f25747fdf1b9286ccaf78e53af421e8 3.33MB / 3.33MB  0.8s
 => => sha256:831704bd2063f9c58ce466588a965a30256b1d6d5 14.84MB / 14.84MB  0.9s
 => => sha256:aff2066ec8914f7383e115bbbcde4d24da428eac3 10.41kB / 10.41kB  0.0s
 => => sha256:d10556fbb8b9849e3d1d281b7bcaad11a7adbeb4583 1.75kB / 1.75kB  0.0s
 => => sha256:d0b3594cb4b0680adee2a52e50eaa169ce8048d3b41 5.30kB / 5.30kB  0.0s
 => => sha256:2d211dd37fa2a9e4524173d4247c8b387b6696e2e36e528 249B / 249B  1.0s
 => => extracting sha256:b16f1b16678093d11ecfece1004207a40f9bc1b7d9d1d16a  2.4s
 => => extracting sha256:8a45c7e905d6f25747fdf1b9286ccaf78e53af421e86800b  0.2s
 => => extracting sha256:831704bd2063f9c58ce466588a965a30256b1d6d54896244  1.3s
 => => extracting sha256:2d211dd37fa2a9e4524173d4247c8b387b6696e2e36e5287  0.0s
 => [ 2/10] WORKDIR /app                                                   1.8s
 => [ 3/10] COPY . /app                                                    0.4s
 => [ 4/10] RUN apt-get update && apt-get install -y     build-essential  38.3s
 => [ 5/10] RUN pip install --upgrade pip                                 13.7s
 => [ 6/10] RUN pip install cached_path==1.6.2 botocore==1.34.88         247.9s
 => [ 7/10] RUN pip install -e .                                         389.5s
 => [ 8/10] RUN python -m unidic download                                 31.8s
 => [ 9/10] RUN pip install nltk                                           5.8s
 => [10/10] COPY nltk_data /app                                            0.1s
 => exporting to image                                                    40.7s
 => => exporting layers                                                   40.7s
 => => writing image sha256:ea131a4df0225d2148d881bc7fbd8aad5a1c4e6c00740  0.0s
 => => naming to docker.io/library/melotts                                 0.0s

root@server:~/melotts.c5/weather.app/MeloTTS.lib# 
```

* Docker 실행 (실행폴더를 공유함)
  - docker run --rm --network=host -it -v $(pwd):/app melotts [in.txt] [out.wav] # 컨테이너 종료시 삭제

* Docker 종료
  - [Ctrl + D]

* 중지된 모든 컨테이너 삭제
  - docker container prune -f

* 사용되지 않는 이미지 삭제
  - docker image prune -a -f

* 사용되지 않는 모든 데이터 삭제 (볼륨, 네트워크 포함 주의!)
  - docker system prune -a -f --volumes

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
// h/w mute disable:q
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
