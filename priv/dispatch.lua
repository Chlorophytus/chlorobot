dofile('priv/version.lua')
dofile('priv/calculator.lua')

commands = {}

help = {
  ping = "replies with a message",
  reload = "reloads the Lua startup script",
  exit = "makes the bot quit gracefully",
  eval = "evaluates a Lua expression",
  version = "shows bot version information",
  join = "joins an IRC channel",
  part = "leaves an IRC channel",
  perms = "handles user permissions for running dangerous commands",
  help = "shows brief help for a specified command",
  calc = "performs calculations in RPN",
}

function commands.ping(hostinfo, destination, arguments)
  chlorobot.respond(destination, hostinfo.nickname .. ": Pong")
end

function commands.reload(hostinfo, destination, arguments)
  if permissions.get(hostinfo.cloak, 'reload') then
    chlorobot.respond(destination, hostinfo.nickname .. ": Reloading script(s)")
    chlorobot.reload()
  end
end

function commands.exit(hostinfo, destination, arguments)
  if permissions.get(hostinfo.cloak, 'exit') then
    chlorobot.respond(destination, hostinfo.nickname .. ": Exiting")
    chlorobot.stop()
  end
end

function commands.eval(hostinfo, destination, arguments)
  if permissions.get(hostinfo.cloak, 'eval') then
    local command = ""

    for i = 1, #arguments do
      if i > 1 then
        command = command .. " " .. arguments[i]
      else
        command = arguments[i]
      end
    end

    local fun, error = load(command)

    if fun ~= nil then
      local ok, inner = pcall(fun)
      if ok then
        if type(inner) ~= "string" then
          chlorobot.respond(destination, hostinfo.nickname .. " - result: " .. tostring(inner))
        else
          chlorobot.respond(destination, hostinfo.nickname .. " - result: " .. inner)
        end
      else
        chlorobot.respond(destination, hostinfo.nickname .. " - execution error: " .. inner)
      end
    else
      chlorobot.respond(destination, hostinfo.nickname .. " - compile error: " .. error)
    end
  end
end

function commands.version(hostinfo, destination, arguments)
  local cv = chlorobot.core_version()
  local core = string.format("%d.%d.%d-r%d", cv.major, cv.minor, cv.patch, cv.revision)
  local sv = chlorobot.script_version
  local script = string.format("%d.%d.%d-r%d", sv.major, sv.minor, sv.patch, sv.revision)
  chlorobot.respond(destination, hostinfo.nickname .. ": Using core v" .. core .. " and script v" .. script)
end

function commands.join(hostinfo, destination, arguments)
  if permissions.get(hostinfo.cloak, 'channels_mut') then
    chlorobot.respond(destination, hostinfo.nickname .. ": Joining " .. arguments[1])
    chlorobot.send({
      command = "JOIN",
      params = {arguments[1]},
    })
  end
end

function commands.part(hostinfo, destination, arguments)
  if permissions.get(hostinfo.cloak, 'channels_mut') then
    chlorobot.respond(destination, hostinfo.nickname .. ": Leaving " .. arguments[1])
    chlorobot.send({
      command = "PART",
      params = {arguments[1]},
    })
  end
end

function commands.perms(hostinfo, destination, arguments)
  if #arguments == 0 then
    local perms = permissions.bulk_get_raw(hostinfo.cloak)
    chlorobot.respond(destination, hostinfo.nickname .. ": You have permissions '" .. perms .. "'")
  elseif #arguments == 1 then
    if permissions.get(hostinfo.cloak, 'perms_get') then
      local user = string.lower(arguments[1])
      local perms = permissions.bulk_get_raw(user)
      chlorobot.respond(destination, hostinfo.nickname .. ": ".. user .. " has permissions '" .. perms .. "'")     
    end
  else
    if permissions.get(hostinfo.cloak, 'perms_mut') then
      local user = string.lower(arguments[1])
      for i = 2, #arguments do
        local perm = arguments[i]
        if string.find(perm, "^-") ~= nil then
          permissions.put(user, string.sub(perm, 2), false)
        elseif string.find(perm, "^+") ~= nil then
          permissions.put(user, string.sub(perm, 2), true)
        end
      end

      local perms = permissions.bulk_get_raw(user)
      chlorobot.respond(destination, hostinfo.nickname .. ": ".. user .. " now has permissions '" .. perms .. "'")     
    end
  end
end

function commands.calc(hostinfo, destination, arguments)
  local ok, result = calculator.calculate(arguments)

  if ok then
    chlorobot.respond(destination, hostinfo.nickname .. ": Calculation result is " .. tostring(result))
  else
    chlorobot.respond(destination, hostinfo.nickname .. ": Calculation error - " .. result)
  end
end


function commands.help(hostinfo, destination, arguments)
  if #arguments == 0 then
    local cmds = {}

    for cmd, _ in pairs(commands) do
      table.insert(cmds, cmd)
    end

    chlorobot.respond(destination, hostinfo.nickname .. ": Commands are '" .. table.concat(cmds, ", ") .. "'")
  elseif #arguments == 1 then
    if help[arguments[1]] ~= nil then
      chlorobot.respond(destination, hostinfo.nickname .. ": " .. help[arguments[1]])
    else
      chlorobot.respond(destination, hostinfo.nickname .. ": No help for that command")
    end
  else
    chlorobot.respond(destination, hostinfo.nickname .. ": Specify zero or one arguments for help")
  end
end
