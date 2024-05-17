has_registered = false
not_joined = true
joined_at = nil

print("Lua started successfully")

function on_recv(packet, wall_clock)
    if packet.command == 903 then
        print("Joined on " .. wall_clock .. " milliseconds")
        joined_at = wall_clock
        has_registered = true
    end
    if packet.command == "PING" then
        chlorobot:send({
            command = "PONG",
            trailing_parameter = packet.trailing_parameter
        })
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