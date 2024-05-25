FROM python:3.12

ADD chloresolve.py .
ADD requirements.txt .
RUN pip install -r requirements.txt

ENTRYPOINT [ "python", "chloresolve.py" ]