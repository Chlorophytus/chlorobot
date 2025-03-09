dofile('priv/version.lua')

commands = {}

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
      chlorobot.respond(destination, hostinfo.nickname .. ": Evaluating")
      fun()
    else
      chlorobot.respond(destination, hostinfo.nickname .. ": " .. error)
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
