local joining = true
local trigger = os.getenv("CHLOROBOT_TRIGGER")
local myself = os.getenv("CHLOROBOT_NICKNAME")
local heartbeat_interval = tonumber(os.getenv("CHLOROBOT_HEALTHCHECK_INTERVAL"))

if chlorobot.last_heartbeat == nil then
  chlorobot.last_heartbeat = 0
end

function chlorobot.log(message)
  local date = os.date("%x %X")

  chlorobot.log_raw(string.format("(%s) %s", date, message))
end

function chlorobot.parse_hostmask(hostmask)
  local nickname, ident, cloak = string.match(hostmask, "(.+)!(.+)@(.+)")

  if nickname ~= nil then
    return {
      nickname = nickname,
      ident = ident,
      cloak = cloak,
    }
  else return nil end
end

function chlorobot.log_message(destination, nickname, message_or_action, is_action)
  if is_action then
    chlorobot.log(string.format("[%s] *** %s %s", destination, nickname, message_or_action))
  else
    chlorobot.log(string.format("[%s] <%s> %s", destination, nickname, message_or_action))
  end
end

function chlorobot.respond(destination, message, do_action)
  local message_or_action = message

  if do_action then
    message_or_action = "\x01ACTION " .. message .. "\x01"
  end

  if string.find(destination, "^" .. "#") ~= nil then
    chlorobot.log_message(destination, myself, message, do_action)
    chlorobot.send({
      command = "PRIVMSG",
      params = {destination},
      trailing_param = message_or_action
    })
  else
    chlorobot.log_message(destination, myself, message, do_action)
    chlorobot.send({
      command = "NOTICE",
      params = {destination},
      trailing_param = message_or_action
    })
  end
end

function chlorobot.get_action(text)
  return string.match(text, "\x01ACTION (.+)\x01")
end

dofile("priv/dispatch.lua")
dofile("priv/health_check.lua")
dofile("priv/permissions.lua")
permissions.initialize(false)

function chlorobot.tick(packet)
  if heartbeat_interval > 0 then
    local this_time = os.time()

    if chlorobot.last_heartbeat + heartbeat_interval < this_time then
      chlorobot.last_heartbeat = this_time
      health_check.heartbeat()
    end
  end

  if packet ~= nil then
    if joining and packet.command == 5 then
      chlorobot.send({
        command = "JOIN",
        params = {
          os.getenv("CHLOROBOT_AUTOJOINS"),
        },
      })
      joining = false
    end

    if packet.command == "PRIVMSG" then
      local host = chlorobot.parse_hostmask(packet.prefix)
      local destination = packet.params[1]
      
      if string.sub(packet.params[1], 1, 1) ~= "#" then
        destination = host.nickname
      end

      local message = packet.trailing_param
      local action = chlorobot.get_action(message)


      if action == nil then
        chlorobot.log_message(destination, host.nickname, message, false)

        if string.find(message, "^" .. trigger) ~= nil then
          local tail = string.sub(message, #trigger + 1)
          local parameters = {}
          for p in string.gmatch(tail, "([^ ]+)") do
            table.insert(parameters, p)
          end

          local call = commands[parameters[1]]
          table.remove(parameters, 1)

          if call ~= nil then
            call(host, destination, parameters)
          else
            chlorobot.respond(destination, host.nickname .. ": Command not found", false)
          end
        end
      else
        chlorobot.log_message(destination, host.nickname, action, true)
      end
    end

    if packet.command == "MODE" then
      local host = chlorobot.parse_hostmask(packet.prefix)

      if host ~= nil then
        local channel_modes = ""

        for i = 2, #packet.params do
          if i > 2 then
            channel_modes = channel_modes .. " " .. packet.params[i]
          else
            channel_modes = packet.params[i]
          end
        end

        chlorobot.log(string.format("[%s] %s sets channel mode(s) '%s'", packet.params[1], host.nickname, channel_modes))
      else
        chlorobot.log(string.format("%s sets user mode(s) '%s'", packet.params[1], packet.trailing_param))
      end 
    end

    if packet.command == "JOIN" then
      local host = chlorobot.parse_hostmask(packet.prefix)
      chlorobot.log(string.format("[%s] %s joins", packet.params[1], host.nickname))
    end

    if packet.command == "PART" then
      local host = chlorobot.parse_hostmask(packet.prefix)
      if packet.trailing_param ~= nil then
        chlorobot.log(string.format("[%s] %s parts (%s)", packet.params[1], host.nickname, packet.trailing_param))
      else
        chlorobot.log(string.format("[%s] %s parts", packet.params[1], host.nickname))
      end
    end

    if packet.command == "QUIT" then
      local host = chlorobot.parse_hostmask(packet.prefix)
      if packet.trailing_param ~= nil then
        chlorobot.log(string.format("%s quits (%s)", host.nickname, packet.trailing_param))
      else
        chlorobot.log(string.format("%s quits", host.nickname))
      end
    end

    if packet.command == "KICK" then
      local host = chlorobot.parse_hostmask(packet.prefix)
      if packet.trailing_param ~= nil then
        chlorobot.log(string.format("[%s] %s kicks %s (%s)", packet.params[1], host.nickname, packet.params[2], packet.trailing_param))
      else
        chlorobot.log(string.format("[%s] %s kicks %s", packet.params[1], host.nickname, packet.params[2]))
      end
    end

    if packet.command == "TOPIC" then
      local host = chlorobot.parse_hostmask(packet.prefix)
      if packet.trailing_param ~= nil then
        chlorobot.log(string.format("[%s] %s sets the topic to '%s'", packet.params[1], host.nickname, packet.trailing_param))
      else
        chlorobot.log(string.format("[%s] %s clears the topic", packet.params[1], host.nickname))
      end
    end
  end
end
