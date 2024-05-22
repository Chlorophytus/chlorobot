function utils_message(chat, message)
    chlorobot:send({
        command = "PRIVMSG",
        parameters = { chat },
        trailing_parameter = message
    })
end