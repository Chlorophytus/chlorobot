local joining = true
local trigger = os.getenv("CHLOROBOT_TRIGGER")

function chlorobot.parse_hostmask(hostmask)
  local nickname, ident, cloak = string.match(hostmask,"(.+)!(.+)@(.+)")

  return {
    nickname = nickname,
    ident = ident,
    cloak = cloak,
  }
end

function chlorobot.get_action(text)
  return string.match(text, "\x01ACTION (.+)\x01")
end

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
      local date = os.date("%x %X")
      local host = chlorobot.parse_hostmask(packet.prefix)
      local destination = packet.params[1]
      local message = packet.trailing_param

      local action = chlorobot.get_action(message)

      if action ~= nil then
        print(string.format("(%s) [%s] *** %s %s", date, destination, host.nickname, action))
      else
        print(string.format("(%s) [%s] <%s> %s", date, destination, host.nickname, message))
      end
    end
  end
end
