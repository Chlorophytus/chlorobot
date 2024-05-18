chlorobot_commands = {}

function command_ping(cloak, reply_fun, args)
    reply_fun("Pong")
end

function command_list(cloak, reply_fun, args)
    local commands = ""
    for k, _ in pairs(chlorobot_commands) do
        commands = commands .. " " .. k
    end

    reply_fun("Possible commands to use are:" .. commands)
end

function command_info(cloak, reply_fun, args)
    reply_fun("I am Chlorobot v" .. chlorobot:version())
end

function command_memory(cloak, reply_fun, args)
    local memory_fd = io.popen("free -m -L")

    if memory_fd ~= nil then
        local memory = memory_fd:read("l")
        memory_fd:close()
        reply_fun("Memory usage data in megabytes: " .. memory)
    else
        reply_fun("Could not get memory usage data")
    end
end

function command_uptime(cloak, reply_fun, args)
    if args[1] ~= "funk" then
        local uptime_fd = io.popen("uptime")

        if uptime_fd ~= nil then
            local uptime = uptime_fd:read("l")
            uptime_fd:close()
            reply_fun("Uptime data: " .. uptime)
        else
            reply_fun("Could not get uptime data")
        end
    else
        reply_fun("Have a lot of fun")
    end
end

function command_fortune(cloak, reply_fun, args)
    local fortune_fd = io.popen("fortune -s")

    if fortune_fd ~= nil then
        local fortune = fortune_fd:read("l")
        fortune_fd:close()
        reply_fun("Fortune: " .. fortune)
    else
        reply_fun("Could not get fortune")
    end
end

function command_reload_cmds(cloak, reply_fun, args)
    if string.lower(cloak) == chlorobot:my_owner() then
        reply_fun("Trying to reload commands")
        dofile("priv/commands.lua")
    else
        reply_fun("Not authorized to reload commands")
    end
end

chlorobot_commands = {
    ping = command_ping,
    list = command_list,
    info = command_info,
    memory = command_memory,
    uptime = command_uptime,
    fortune = command_fortune,
    reload_cmds = command_reload_cmds
}
