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

// Docker 엔진설치 (docker.io는 더이상 사용하지 않으므로 이미 설치하였다면 삭제하여야 함)
// 기존 docker.io삭제
// # for pkg in docker.io docker-doc docker-compose docker-compose-v2 podman-docker containerd runc; do sudo apt-get remove $pkg; done
// # apt autoremove
root@server:~# apt update && apt install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

// Docker network(iptable) 관련설정이 없거나 지원하지 않는 경우(ODROID-C5는 지원하지 않음)
// 이 경우 docker실행시 --network=host명령을 주어 host의 network을 사용하도록 함.
// update 20250617 : 최신 커널에는 반영됨. module list가 modprobe.d에 설정되어야 함(모듈 파일작성 내용 참고)
root@server:~# mkdir -p /etc/docker
root@server:~# tee /etc/docker/daemon.json > /dev/null <<EOF
{
  "iptables": false
}
EOF

// system reboot
root@server:~# reboot

//
// FB 나오지 않는 경우(필요한 package설치 및 drm관련 모니터 설정함)
//
root@server:~# sudo apt install libdrm-meson libdrm-meson-dev -y
root@server:~# drm_setcrtc -d meson -m 1920x720p60hz
root@server:~# drm_setcrtc -d meson -m 800x480p60hz
root@server:~# echo 0 | sudo tee /sys/devices/platform/drm-subsystem/graphics/fb0/blank

root@server:~# uname -a
Linux server 5.15.153-odroid-arm64 #1 SMP PREEMPT Tue, 10 Jun 2025 05:13:57 +0000 aarch64 aarch64 aarch64 GNU/Linux

//
// Docker지원 버전 (iptable관련 포함, Docker build시 host option제거, daemon.json파일 생성 필요 없음)
//
root@server:~# apt update
root@server:~# apt install linux-image-5.15.153-odroid-arm64
root@server:~# dpkg -l | grep linux-image-5.15
ii  linux-image-5.15.153-odroid-arm64 5.15.153-202506121403~noble             arm64        Linux 5.15 for ODROID (64-bit ARMv8 machines)

root@server:~# uname -a
Linux server 5.15.153-odroid-arm64 #1 SMP PREEMPT Thu, 12 Jun 2025 05:14:22 +0000 aarch64 aarch64 aarch64 GNU/Linux

// /etc/modules-load.d/docker.conf 파일 생성 및 아래 모듈 등록
root@server:~# vi /etc/modules-load.d/docker.conf
ip_tables
nf_tables
nf_conntrack

iptable_filter
ip6table_filter
iptable_mangle
ip6table_mangle
iptable_nat
ip6table_nat
iptable_raw
ip6table_raw

br_netfilter

xfrm_user
xt_tcp
xt_udp
xt_limit
xt_MASQUERADE
xt_conntrack
xt_addrtype


// iptable관련 업데이트
root@server:~# apt update
root@server:~# usermod -a -G docker odroid
root@server:~# update-alternatives --set iptables /usr/sbin/iptables-legacy
root@server:~# update-alternatives --set ip6tables /usr/sbin/ip6tables-legacy
root@server:~# reboot

// docker 동작 확인
root@server:~# cat /var/log/syslog | grep dockerd
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
* MeloTTS Library 설치
* Github 주소 : https://github.com/myshell-ai/MeloTTS (https://github.com/myshell-ai/MeloTTS/blob/main/docs/install.md)
```
// MeloTTS Library clone
root@server:~# git clone https://github.com/myshell-ai/MeloTTS MeloTTS.lib
```
    
* Docker 파일 수정 및 추가사항 업데이트 (melotts.c5/library.update folder안에 있는 모든 파일을 MeloTTS lib폴더안에 복사한다.)
* Docker파일 수정내용
  - RUN pip install --upgrade pip
  - RUN pip install cached_pa#
  - RUN python melo/init_downloads.py (주석처리 : 미리 다운로드, docker생성시 다운로드하는 경우 완료되지 않는 경우가 발생)
  - RUN pip install nltk
  - COPY nltk_data /app
  - ENV NLTK_DATA=/app/nltk_data
  - ENTRYPOINT ["python", "./mk_speech.py"]

* RUN python melo/init_downloads.py를 주석처리(리소스부족의 문제로 nlk_data는 먼저 다운로드 하도록한다.)
```
#
# ARM64에서 Docker image 작업중 nlk_data 다운로드시 리소스 문제로 정상적으로 동작하지 않는 경우가 생김
#
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

root@server:~/MeloTTS.lib# pip install nltk
root@server:~/MeloTTS.lib# mkdir -p ./nltk_data
root@server:~/MeloTTS.lib# python3 -m nltk.downloader -d ./nltk_data averaged_perceptron_tagger cmudict punkt wordnet
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

* Docker 실행 (실행폴더를 공유함, 컨테이너 종료시 삭제옵션 포함)
  - docker run --rm --network=host -it -v $(pwd):/app melotts {options}

```
usage: mk_speech.py [-h] [-i [INPUT ...]] [-o [OUTPUT ...]] [-l [LANGUAGE ...]]
                    [-s [SPEED ...]]

mk_speech.py options

optional arguments:
  -h, --help            show this help message and exit
  -i [INPUT ...], --input [INPUT ...]
                        input filename : speech text
  -o [OUTPUT ...], --output [OUTPUT ...]
                        output filename {'default.wav'} : speech wav
  -l [LANGUAGE ...], --language [LANGUAGE ...]
                        language {'EN'} : speech language ['EN' or 'KR]
  -s [SPEED ...], --speed [SPEED ...]
                        speed {1.0} : speech speed
    
```
    
```
root@server:~/melotts.c5/weather.app/MeloTTS.lib# docker run --rm --network=host -it -v $(pwd):/app melotts
tokenizer_config.json: 100%|███████████████████| 251/251 [00:00<00:00, 28.2kB/s]
vocab.txt: 100%|█████████████████████████████| 231k/231k [00:00<00:00, 1.29MB/s]
tokenizer_config.json: 100%|█████████████████| 48.0/48.0 [00:00<00:00, 6.91kB/s]
config.json: 100%|██████████████████████████████| 570/570 [00:00<00:00, 339kB/s]
vocab.txt: 100%|█████████████████████████████| 232k/232k [00:00<00:00, 11.2MB/s]
tokenizer.json: 100%|████████████████████████| 466k/466k [00:00<00:00, 2.53MB/s]
tokenizer_config.json: 100%|█████████████████| 48.0/48.0 [00:00<00:00, 28.2kB/s]
config.json: 100%|██████████████████████████████| 625/625 [00:00<00:00, 409kB/s]
vocab.txt: 100%|█████████████████████████████| 872k/872k [00:00<00:00, 4.38MB/s]
tokenizer.json: 100%|██████████████████████| 1.72M/1.72M [00:00<00:00, 8.77MB/s]
tokenizer_config.json: 100%|█████████████████| 80.0/80.0 [00:00<00:00, 48.7kB/s]
config.json: 100%|██████████████████████████████| 725/725 [00:00<00:00, 453kB/s]
vocab.txt: 100%|█████████████████████████████| 344k/344k [00:00<00:00, 1.89MB/s]
tokenizer_config.json: 100%|█████████████████| 83.0/83.0 [00:00<00:00, 12.0kB/s]
config.json: 100%|██████████████████████████████| 420/420 [00:00<00:00, 253kB/s]
vocab.txt: 100%|█████████████████████████████| 227k/227k [00:00<00:00, 61.9MB/s]
tokenizer_config.json: 100%|███████████████████| 310/310 [00:00<00:00, 71.4kB/s]
config.json: 100%|██████████████████████████████| 650/650 [00:00<00:00, 399kB/s]
vocab.txt: 100%|█████████████████████████████| 248k/248k [00:00<00:00, 1.36MB/s]
tokenizer.json: 100%|████████████████████████| 486k/486k [00:00<00:00, 37.1MB/s]
special_tokens_map.json: 100%|█████████████████| 134/134 [00:00<00:00, 81.9kB/s]
***********************************
input text filename =  
output wav filename =  error.wav
***********************************
날씨 정보가 없습니다.
***********************************
config.json: 100%|██████████████████████████| 3.40k/3.40k [00:00<00:00, 490kB/s]
/usr/local/lib/python3.9/site-packages/torch/nn/utils/weight_norm.py:143: FutureWarning: `torch.nn.utils.weight_norm` is deprecated in favor of `torch.nn.utils.parametrizations.weight_norm`.
  WeightNorm.apply(module, name, dim)
checkpoint.pth: 100%|████████████████████████| 208M/208M [00:02<00:00, 99.3MB/s]
 > Text split to sentences.
날씨 정보가 없습니다.
 > ===========================
  0%|                                                     | 0/1 [00:00<?, ?it/s]you have to install python-mecab-ko. install it...
huggingface/tokenizers: The current process just got forked, after parallelism has already been used. Disabling parallelism to avoid deadlocks...
To disable this warning, you can either:
	- Avoid using `tokenizers` before the fork if possible
	- Explicitly set the environment variable TOKENIZERS_PARALLELISM=(true | false)
Collecting python-mecab-ko
  Downloading python_mecab_ko-1.3.7-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl.metadata (3.4 kB)
Collecting python-mecab-ko-dic (from python-mecab-ko)
  Downloading python_mecab_ko_dic-2.1.1.post2-py3-none-any.whl.metadata (1.4 kB)
Downloading python_mecab_ko-1.3.7-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl (560 kB)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ 560.6/560.6 kB 13.7 MB/s eta 0:00:00
Downloading python_mecab_ko_dic-2.1.1.post2-py3-none-any.whl (34.5 MB)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ 34.5/34.5 MB 26.8 MB/s eta 0:00:00
Installing collected packages: python-mecab-ko-dic, python-mecab-ko
Successfully installed python-mecab-ko-1.3.7 python-mecab-ko-dic-2.1.1.post2
WARNING: Running pip as the 'root' user can result in broken permissions and conflicting behaviour with the system package manager, possibly rendering your system unusable. It is recommended to use a virtual environment instead: https://pip.pypa.io/warnings/venv. Use the --root-user-action option if you know what you are doing and want to suppress this warning.
pytorch_model.bin: 100%|█████████████████████| 476M/476M [00:04<00:00, 97.5MB/s]
Some weights of the model checkpoint at kykim/bert-kor-base were not used when initializing BertForMaskedLM: ['cls.seq_relationship.weight', 'cls.seq_relationship.bias']
- This IS expected if you are initializing BertForMaskedLM from the checkpoint of a model trained on another task or with another architecture (e.g. initializing a BertForSequenceClassification model from a BertForPreTraining model).
- This IS NOT expected if you are initializing BertForMaskedLM from the checkpoint of a model that you expect to be exactly identical (initializing a BertForSequenceClassification model from a BertForSequenceClassification model).
100%|█████████████████████████████████████████████| 1/1 [00:40<00:00, 40.25s/it]

# play test
root@server:~/melotts.c5/weather.app/MeloTTS.lib# aplay -Dplughw:0,2 error.wav 
Playing WAVE 'error.wav' : Signed 16 bit Little Endian, Rate 44100 Hz, Mono

```

* Docker 종료 (Docker Bash모드의 경우)
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
