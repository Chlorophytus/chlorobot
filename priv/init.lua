dofile("priv/commands.lua")
print("Lua started successfully")

function on_recv(packet)
    -- SASL Success
    if packet.command == 900 then
        dofile("priv/env.lua")
    end
    -- Pings/pongs
    if packet.command == "PING" then
        chlorobot:send({
            command = "PONG",
            trailing_parameter = packet.trailing_parameter
        })
    end
    -- Commands
    if packet.command == "PRIVMSG" then
        local bang_token = string.find(packet.prefix, "!", 1, true)
        local at_token = string.find(packet.prefix, "@", 1, true)
        local username = string.sub(packet.prefix, 1, bang_token - 1)
        local cloak = string.sub(packet.prefix, at_token + 1)

        local origin = packet.parameters[1]
        local use_notices = origin == chlorobot:my_username()
        local trailing = packet.trailing_parameter

        if string.sub(trailing, 1, 2) == "c|" then
            local command = ""
            local args = {}
            local i = 0
            for v in string.gmatch(trailing, "%S+", 3) do
                if i == 0 then
                    command = v
                else
                    args[i] = v
                end
                i = i + 1
            end
            local not_present = true
            local reply_fun = function(text)
                chlorobot:send({
                    command = "PRIVMSG",
                    parameters = { origin },
                    trailing_parameter = username .. ": " .. text
                })
            end
            if use_notices then
                reply_fun = function(text)
                    chlorobot:send({
                        command = "NOTICE",
                        parameters = { username },
                        trailing_parameter = text
                    })
                end
            end

            for key, value in pairs(chlorobot_commands) do
                if key == command then
                    not_present = false
                    value(cloak, reply_fun, args)
                end
            end

            if not_present then
                reply_fun("Command not found")
            end
        end
    end
end
