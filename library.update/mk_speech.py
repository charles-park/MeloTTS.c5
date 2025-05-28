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
# TEXT Read function
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
import argparse

parser = argparse.ArgumentParser(description="mk_speech.py options")
parser.add_argument("-i", "--input", nargs='*',  type=str, default="", help="input filename  : speech text")
parser.add_argument("-o", "--output", nargs='*', type=str, default="default.wav", help="output filename {'default.wav'} : speech wav")
parser.add_argument("-l", "--language", nargs='*', type=str, default="EN", help="language {'EN'} : speech language ['EN' or 'KR]")
parser.add_argument("-s", "--speed", nargs='*', type=float, default=1.0, help="speed {1.0} : speech speed")

args = parser.parse_args()

input_fname = args.input

if (args.language.upper() == "KR"):
    language   = "KR"
    speaker_id = "KR"
else:
    language   = "EN"
    speaker_id = "EN-US"

output_fname = args.output
speed = args.speed

#------------------------------------------------------------------------------
#
# File read
#
#------------------------------------------------------------------------------
read_txt = read_text_file(input_fname, language)

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
#
# Input param & read text print
#
#------------------------------------------------------------------------------
print("---------------------------------------")
print("[ input param ]")
print("language = ", language)
print("speaker id = ", speaker_id)
print("speech speed = ", speed)
print("input text filename = ", input_fname)
print("output wav filename = ", output_fname)
print("---------------------------------------")
print("[ speech text ]")
print(read_txt)
print("---------------------------------------")

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

