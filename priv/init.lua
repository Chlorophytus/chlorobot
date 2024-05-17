has_registered = false
not_joined = true
joined_at = nil

dofile("priv/commands.lua")
print("Lua started successfully")

function on_recv(packet, wall_clock)
    -- SASL Success
    if packet.command == 903 then
        print("Joined within " .. wall_clock .. "ms")
        joined_at = wall_clock
        has_registered = true
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
        local username = string.sub(packet.prefix, 1, bang_token - 1)
        local origin = packet.parameters[1]
        local use_notices = origin == "Chlorobot" 
        local trailing = packet.trailing_parameter

        if string.sub(trailing, 1, 2) == "c|" then
            local command = ""
            local args = {}
            local i = 0
            for v in string.gmatch(trailing, "%g+", 3) do
                if i == 0 then
                    command = v
                else
                    args[i] = v
                end
                i = i + 1
            end
            local not_present = true
            local reply_fun = function (text)
                chlorobot:send({
                    command = "PRIVMSG",
                    parameters = { origin },
                    trailing_parameter = username .. ": " .. text
                })
            end
            if use_notices then
                reply_fun = function (text)
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
                    value(username, reply_fun, args)
                end
            end

            if not_present then
                reply_fun("Command not found")
            end
        end
    end
end

function on_tick(wall_clock)
    if has_registered then
        if not_joined and (wall_clock - joined_at) > 1000 then
            not_joined = false
            dofile("priv/env.lua")
        end
    end
end
