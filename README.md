# chlorobot
C++ IRC bot with a Lua command handler


## Environment files

At the bare minimum, Chlorobot requires a `.env` file.
```
CHLOROBOT_NICKNAME=ExampleBotNick
CHLOROBOT_IDENT=ExampleIdent
CHLOROBOT_REALNAME=Example Bot Real Name
CHLOROBOT_SASL_USERNAME=your-botuser-here
CHLOROBOT_SASL_PASSWORD=your-password-here
CHLOROBOT_NETWORK_HOST=irc.libera.chat
CHLOROBOT_NETWORK_PORT=6697
CHLOROBOT_OWNER=owner-irc-cloak-here
CHLOROBOT_TRIGGER=ex|
CHLOROBOT_HEALTHCHECK_URI=https://healthcheck.example
CHLOROBOT_HEALTHCHECK_INTERVAL=120
```

## Docker commands

Starting the bot with env file in `./.env`, SQLite tables in `./tables`, and Lua
scripts in `./priv`
```shell
$ docker run -it --env-file .env -v ./tables:/srv/chlorobot/tables -v ./priv:/srv/chlorobot/priv:ro chlorobot:latest
```
