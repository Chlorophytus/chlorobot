# chlorobot
C++ IRC bot with a gRPC-connected Python core


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
CHLOROBOT_RPC_SERVER=chlorobot-rpc
```

You can load that in Docker Compose, `CHLOROBOT_RPC_SERVER` should be
`chlorobot-rpc` unless the RPC server variable is actually edited in the
`docker-compose.yml` file.

## Docker Compose commands

Chloresolver is just an example of a Python resolver that connects to the C++
core's gRPC endpoint. By default, the Chlorobot gRPC server is assigned hostname
`chlorobot-rpc`.

> [!IMPORTANT]
> The bot core will bind to `0.0.0.0` by default for gRPC, so use a firewall.
> 
> Future bot core versions might make this an environment variable.

### Managing the bot

#### Starting its C++ core and all dependencies

C++ core updates are rare, unless something arises.

```shell
$ docker-compose build # if you need to rebuild it
$ docker-compose up
```

#### Hot swapping the Python resolver

The Python resolver can be swapped out with a different resolver, but here are
the commands you need to rebuild and restart the resolver.

The core will ping the IRC server, so the connection should not go down.

```shell
$ docker-compose down chloresolver # if it hasn't crashed
$ docker-compose up -d --build chloresolver
```