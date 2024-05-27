FROM python:3.12

ADD requirements.txt .
RUN pip install -r requirements.txt

ADD main.py .
ADD chlorobot_rpc_pb2.pyi .
ADD chlorobot_rpc_pb2.py .
ADD chlorobot_rpc_pb2_grpc.py .
ADD chloresolve/* ./chloresolve/
ADD corpus_data/* ./corpus_data/

ENTRYPOINT [ "python", "main.py" ]