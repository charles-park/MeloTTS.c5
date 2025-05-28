#------------------------------------------------------------------------------
'''
/**
 * @file mk_speech.py
 * @author charles-park (charles.park@hardkernel.com)
 * @brief This Application is make the speech audio file(wav). run within the docker image.
 * @version 2.0
 * @date 2025-05-28
 *
 * @package apt install libcurl4-openssl-dev libcjson-dev
 *
 * @copyright Copyright (c) 2022
 *
 */
'''
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
import sys
import os

from melo.api import TTS

#------------------------------------------------------------------------------
import nltk
from nltk.tag import PerceptronTagger
import pickle  # pickle 임포트 필요함!

#------------------------------------------------------------------------------
#
# pickle read error fix.
#
#------------------------------------------------------------------------------
class PatchedPerceptronTagger(PerceptronTagger):
    def __init__(self):
        pickle_path = "/app/nltk_data/taggers/averaged_perceptron_tagger/averaged_perceptron_tagger.pickle"
        if not os.path.exists(pickle_path):
            raise FileNotFoundError(f"Cannot find tagger pickle at {pickle_path}")

        with open(pickle_path, "rb") as f:
            model_obj = pickle.load(f)

        # case 1: it is an instance of PerceptronTagger
        if isinstance(model_obj, PerceptronTagger):
            tagger = model_obj

        # case 2: it is a tuple (model, tagdict, classes)
        elif isinstance(model_obj, tuple) and len(model_obj) == 3:
            from nltk.tag.perceptron import PerceptronTagger as BaseTagger
            tagger = BaseTagger(load=False)
            tagger.model, tagger.tagdict, tagger.classes = model_obj

        else:
            raise TypeError(f"Unexpected pickle content type: {type(model_obj)}")

        # 설정
        self.model = tagger.model
        self.classes = tagger.classes
        self.tagdict = getattr(tagger, "tagdict", {})
        self._taggers = [tagger]

#------------------------------------------------------------------------------
#
# NLTK monkey patch : PatchedPerceptronTagger() 함수 선언 뒤에실행.
#
#------------------------------------------------------------------------------
nltk.tag._get_tagger = lambda lang=None: PatchedPerceptronTagger()

#------------------------------------------------------------------------------
#
# TEXT Read
#
#------------------------------------------------------------------------------
def read_text_file(filename, lang):
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            return f.read()
    except FileNotFoundError:
        if (lang == 'EN'):
            return "There is not the weather information."
        else :
            return "날씨 정보가 없습니다."

#------------------------------------------------------------------------------
#
# Start Application (argument parsing)
# argu info : 1st = input(text), 2nd = output(wav), 3rd = language ("EN" or "KR")
#
#------------------------------------------------------------------------------
# default language
language = "EN"
speaker_id = "EN-US"
output_fname = "default.wav"
input_fname = ""
speed = 1.0

# input text
if len(sys.argv) >= 2:
    input_fname  = sys.argv[1]
else:
    print ("Usage : mk_speech {in_text=default msg} {out_text=default.wav} {language='EN'} {speed=1.0}")

# output wav
if len(sys.argv) >= 3:
    output_fname = sys.argv[2]

# speech language
if len(sys.argv) >= 4:
    if sys.argv[3].upper() == "KR":
        language = "KR"
        speaker_id = "KR"

# speech language
if len(sys.argv) >= 5:
    try:
        speed = float (sys.argv[4])
    except ValueError:
        speed = 1.0

#------------------------------------------------------------------------------
#
# Input param & read text print
#
#------------------------------------------------------------------------------
print("***********************************")
print("language = ", language)
print("speaker id = ", speaker_id)
print("speech speed = ", speed)
print("input text filename = ", input_fname)
print("output wav filename = ", output_fname)
print("***********************************")
read_txt = read_text_file(input_fname, language)
print("speech text")
print(read_txt)
print("***********************************")

#------------------------------------------------------------------------------
# Local download
# python3 -m nltk.downloader -d ./nltk_data averaged_perceptron_tagger cmudict punkt wordnet
#------------------------------------------------------------------------------
# model = TTS(language='EN', device=device)
# speaker_id = 'EN-US': 0, 'EN-BR': 1, 'EN_INDIA': 2, 'EN-AU': 3, 'EN-Default': 4}
#------------------------------------------------------------------------------
# model = TTS(language='EN', device=device)
# speaker_id = 'KR' = TTS(language='KR', device=device)
#------------------------------------------------------------------------------
# Speed is adjustable
device = 'cpu' # or cuda:0

model = TTS(language, device=device)
speaker_ids = model.hps.data.spk2id
model.tts_to_file(read_txt, speaker_ids[speaker_id], output_fname, speed=speed)

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

