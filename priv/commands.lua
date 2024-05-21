chlorobot_loaded_on = os.date("!%c")
chlorobot_commands = {}

dofile("priv/markov.lua")

function command_ping(cloak, reply_fun, args)
    reply_fun("Pong")
end

function command_help(cloak, reply_fun, args)
    local commands = ""
    for k, _ in pairs(chlorobot_commands) do
        commands = commands .. " " .. k
    end

    reply_fun("Possible commands to use are:" .. commands)
end

function command_version(cloak, reply_fun, args)
    reply_fun("I am Chlorobot v" .. chlorobot:version() .. " (loaded on " .. chlorobot_loaded_on .. ")")
end

function command_memory(cloak, reply_fun, args)
    local memory_fd = io.popen("free -m")

    if memory_fd ~= nil then
        local _        = memory_fd:read("l")
        local memory   = memory_fd:read("l")

        local region   = 1
        local used_mb  = nil
        local total_mb = nil

        for memory_tab in string.gmatch(memory, "%S+") do
            if region > 1 then
                if region == 2 then
                    total_mb = memory_tab
                elseif region == 3 then
                    used_mb = memory_tab
                else
                    break
                end
            end
            region = region + 1
        end

        memory_fd:close()
        reply_fun("Memory usage data: " .. used_mb .. "MB out of " .. total_mb .. "MB used")
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

function command_reload(cloak, reply_fun, args)
    if string.lower(cloak) == chlorobot:my_owner() then
        reply_fun("Trying to reload commands")
        dofile("priv/commands.lua")
    else
        reply_fun("Not authorized to reload commands")
    end
end

function command_cpu_model(cloak, reply_fun, args)
    local cpu_model_fd = io.popen("lscpu | grep \"Model name\" | tr -s ' '")

    if cpu_model_fd ~= nil then
        local cpu_model = cpu_model_fd:read("l")
        cpu_model_fd:close()
        reply_fun("I run on: " .. cpu_model)
    else
        reply_fun("Could not get CPU model")
    end
end

function command_markov(cloak, reply_fun, args)
    -- For example, using the NLTK Inaugural dataset needs to be sanitized:
    -- cat *.txt | sed 's/^[[:blank:]]*//;s/[[:blank:]]*$//' | sed '/^$/d' | tr -cd '\11\12\15\40-\176' > inaugural.txt
    -- then in IRC: "c|markov load inaugural.txt"
    if args[1] == "load" then
        if string.lower(cloak) == chlorobot:my_owner() then
            if #args > 1 then
                table.remove(args, 1)
                local corpus_name = table.concat(args, " ")
                reply_fun("Trying to load a text corpus: " .. corpus_name)
                chlorobot_markov_data = markov_feed(corpus_name)
            else
                reply_fun("Usage: markov load (file_name...)")
            end
        else
            reply_fun("Not authorized to load a Markov text corpus")
        end
    end
    if args[1] == "run" then
        if string.lower(cloak) == chlorobot:my_owner() then
            if #args == 3 then
                local min = tonumber(args[2])
                local max = tonumber(args[3])
                reply_fun(markov_run(chlorobot_markov_data, min, max))
            else
                reply_fun("Usage: markov run (min_words) (max_words)")
            end
        else
            reply_fun("Not authorized to run the Markov chain text generator")
        end
    end
end

chlorobot_commands = {
    ping = command_ping,
    help = command_help,
    version = command_version,
    memory = command_memory,
    uptime = command_uptime,
    fortune = command_fortune,
    reload = command_reload,
    cpu_model = command_cpu_model,
    markov = command_markov
}
