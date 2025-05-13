FROM python:3.9-slim
WORKDIR /app
COPY . /app

RUN apt-get update && apt-get install -y \
    build-essential libsndfile1 \
    && rm -rf /var/lib/apt/lists/*

RUN pip install --upgrade pip
RUN pip install cached_path==1.1.3 botocore==1.29.76

RUN pip install -e .
RUN python -m unidic download
RUN python melo/init_downloads.py

CMD ["python", "./mk_speech.py"]
# CMD ["/bin/bash"]
