import sys
from melo.api import TTS

def read_text_file(filename, default="날씨 정보가 없습니다."):
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            return f.read()
    except FileNotFoundError:
        return default

# 예시
# kr_txt = read_text_file("kr.txt")
# print(kr_txt)
if len(sys.argv) < 2:
    input_fname = ""
    output_fname = "error.wav"
else:
    input_fname = sys.argv[1]
    if len (sys.argv) == 3:
        output_fname = sys.argv[2]

print("***********************************")
print("input text filename = ", input_fname)
print("output wav filename = ", output_fname)
print("***********************************")
kr_txt = read_text_file(input_fname)
print(kr_txt)
print("***********************************")

# Speed is adjustable
speed = 1.0
device = 'cpu' # or cuda:0

model = TTS(language='KR', device=device)
speaker_ids = model.hps.data.spk2id

# output_path = 'kr.wav'
# model.tts_to_file(kr_txt, speaker_ids['KR'], output_path, speed=speed)
model.tts_to_file(kr_txt, speaker_ids['KR'], output_fname, speed=speed)

