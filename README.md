# chlorobot
IRC Bot


## Environment files

At the bare minimum, Chlorobot requires a `.env` file. You can set the RPC
token to anything, just keep it safe.
```bash
export CHLOROBOT_NICKNAME="ExampleBotNick"
export CHLOROBOT_IDENT="ExampleIdent"
export CHLOROBOT_REALNAME="Example Bot Real Name"
export CHLOROBOT_SASL_USERNAME="your-botuser-here"
export CHLOROBOT_SASL_PASSWORD="your-password-here"
export CHLOROBOT_NETWORK_HOST="irc.libera.chat"
export CHLOROBOT_NETWORK_PORT="6697"
export CHLOROBOT_OWNER="owner-irc-cloak-here"
export CHLOROBOT_RPC_TOKEN="random_value_for_rpc_token"
```

Then do `. .env`.

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
