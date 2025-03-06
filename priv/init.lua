dofile("priv/permissions.lua")

permissions.initialize(false)

local joining = true
local trigger = os.getenv("CHLOROBOT_TRIGGER")
local myself = os.getenv("CHLOROBOT_NICKNAME")

function chlorobot.parse_hostmask(hostmask)
  local nickname, ident, cloak = string.match(hostmask,"(.+)!(.+)@(.+)")

  return {
    nickname = nickname,
    ident = ident,
    cloak = cloak,
  }
end

function chlorobot.log_message(destination, nickname, message_or_action, is_action)
  local date = os.date("%x %X")
  if is_action then
    print(string.format("(%s) [%s] *** %s %s", date, destination, nickname, message_or_action))
  else
    print(string.format("(%s) [%s] <%s> %s", date, destination, nickname, message_or_action))
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

function chlorobot.tick(packet)
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
      local message = packet.trailing_param
      local action = chlorobot.get_action(message)

      if action == nil then
        chlorobot.log_message(destination, host.nickname, message, false)

        if string.find(message, "^" .. trigger) ~= nil then
          local tail = string.sub(message, #trigger + 1)
          local parameters = {}
          for p in string.gmatch(tail, "([^,]+)") do
            table.insert(parameters, p)
          end

          local call = commands[parameters[1]]
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
  end
end
