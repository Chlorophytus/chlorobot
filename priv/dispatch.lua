commands = {}

function commands.ping(hostinfo, destination, arguments)
  chlorobot.respond(destination, hostinfo.nickname .. ": Pong")
end
