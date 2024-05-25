# chlorobot
IRC Bot


## Environment files

At the bare minimum, Chlorobot requires a `.env` file. You can set the RPC
token to anything, just keep it safe.
```
CHLOROBOT_NICKNAME=ExampleBotNick
CHLOROBOT_IDENT=ExampleIdent
CHLOROBOT_REALNAME=Example Bot Real Name
CHLOROBOT_SASL_USERNAME=your-botuser-here
CHLOROBOT_SASL_PASSWORD=your-password-here
CHLOROBOT_NETWORK_HOST=irc.libera.chat
CHLOROBOT_NETWORK_PORT=6697
CHLOROBOT_OWNER=owner-irc-cloak-here
CHLOROBOT_RPC_TOKEN=random_value_for_rpc_token
```

You can load that in Docker

## Build on Ubuntu

For a default minimal configuration, install these packages

```shell
$ sudo apt install build-essential cmake libssl-dev libtls-dev libgrpc++-dev protobuf-compiler-grpc
```

```shell
$ mkdir build
$ cmake -B build/ .
$ make -C build/
```

Generate example Python3 protobufs and run example resolver...
```shell
$ cd chloresolve
$ virtualenv venv
$ . venv/bin/activate
$ pip install -r requirements.txt
$ python3 -m grpc_tools.protoc ../chlorobot_rpc.proto --proto_path=.. --python_out=./ --grpc_python_out=./ --pyi_out=./
$ python3 chloresolve.py
```
