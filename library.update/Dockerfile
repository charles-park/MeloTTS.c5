#
# docker build --no-cache --network=host -t melotts .
#
# docker run --rm --network=host -it -v $(pwd):/app melotts [in.txt] [out.wav] # 컨테이너 종료시 삭제
#
# docker system prune -a -f --volumes
#
FROM python:3.9-slim
WORKDIR /app
COPY . /app

RUN apt-get update && apt-get install -y \
    build-essential libsndfile1 \
    && rm -rf /var/lib/apt/lists/*

RUN pip install --upgrade pip
RUN pip install cached_path==1.6.2 botocore==1.34.88

RUN pip install -e .
RUN python -m unidic download

#
# ARM64에서 Docker image작업중 다운로드시 리소스 문제로 정상적으로 동작하지 않는 경우가 생김
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
#
RUN pip install nltk
COPY nltk_data /app
ENV NLTK_DATA=/app/nltk_data

# CMD ["/bin/bash"]
ENTRYPOINT ["python", "./mk_speech.py"]
