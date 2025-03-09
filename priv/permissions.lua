local sqlite = require('sqlite')

permissions = { db = sqlite{ uri = "tables/ptab.db" } }

function permissions.is_legal(check_permission)
  local legality = 0
  local function check_char(check_character)
    -- check for legal chars one by one, Lua doesn't give us a frozenset type
    for checker in string.gmatch("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ".") do
      if checker == check_character then
        return true
      end
    end

    -- illegal char, so return
    return false
  end

  -- loop through all chars and give a count of how many legal chars there are
  for char in string.gmatch(check_permission, ".") do
    if check_char(char) then
      legality = legality + 1
    end
  end

  -- returns true if legal char count is equal to the permission's char count
  return legality == #check_permission
end

function permissions.bulk_get_raw(raw_cloak)
  local cloak = string.lower(raw_cloak)

  permissions.db:open()
  local entry = permissions.db:select("ptab", { where = { cloak = cloak } })
  permissions.db:close()

  local entry_perms = ""
  if #entry == 1 then
    entry_perms = entry[1].perms
  end

  return entry_perms
end

function permissions.bulk_get(raw_cloak)
  local cloak = string.lower(raw_cloak)
  local entry_perms = permissions.bulk_get_raw(cloak)

  chlorobot.log("permissions bulk_get (" .. cloak .. ") -> " .. entry_perms)
  local split = {}

  for perm in string.gmatch(entry_perms, "([^,]+)") do
    table.insert(split, perm)
  end

  return split
end

function permissions.get(raw_cloak, perm)
  local cloak = string.lower(raw_cloak)
  if permissions.is_legal(perm) then
    local bulk = permissions.bulk_get(cloak)

    for i = 1, #bulk do
      local this_perm = bulk[i]
      if this_perm == perm then
        chlorobot.log("permissions get (" .. cloak .. "): " .. perm .. " -> has permission")
        return true
      elseif this_perm == "_ALL" then
        chlorobot.log("permissions get (" .. cloak .. "): " .. perm .. " -> has _ALL")
        return true
      end
    end

    chlorobot.log("permissions get (" .. cloak .. "): " .. perm .. " -> does not have permission")
    return false
  else
    chlorobot.log("permissions get (" .. cloak .. "): " .. perm .. " -> illegal permission specified")
    return false
  end
end

function permissions.put(raw_cloak, perm, state)
  local cloak = string.lower(raw_cloak)
  if string.sub(perm,1, 1) ~= "_" and permissions.is_legal(perm) then
    local bulk = permissions.bulk_get(cloak)
    local perm_at = nil

    for i = 1, #bulk do
      local this_perm = bulk[i]
      if string.sub(this_perm, 1, 1) == "_" then
        chlorobot.log("permissions put (" .. cloak .. "): " .. perm .. " -> user has '_' sticky permission prefix")
        return
      elseif this_perm == perm then
        if state then
          chlorobot.log("permissions put (" .. cloak .. "): " .. perm .. " SET -> user already has permission")
          return
        elseif perm_at == nil then
          perm_at = i
        end
      end
    end

    if state then
      table.insert(bulk, perm)
      chlorobot.log("permissions put (" .. cloak .. "): SET " .. perm .. " -> OK")
    elseif perm_at ~= nil then
      table.remove(bulk, perm_at)
      chlorobot.log("permissions put (" .. cloak .. "): CLEAR " .. perm .. " -> OK")
    else
      chlorobot.log("permissions put (" .. cloak .. "): CLEAR " .. perm .. " -> user doesn't have permission")
      return
    end

    local perms_string = ""
    for i, v in ipairs(bulk) do
      if i > 1 then
        perms_string = perms_string .. "," .. v
      else
        perms_string = v
      end
    end

    permissions.db:open()
    permissions.db:update("ptab", { where = { cloak = cloak }, set = { perms = perms_string } })
    permissions.db:close()
  else
    chlorobot.log("permissions put (" .. cloak .. "): " .. perm .. " -> illegal or sticky permission specified")
  end
end

function permissions.initialize(force)
  permissions.db:open()
  if force then
    permissions.db:drop("ptab")
  end
  local ptab = permissions.db:tbl("ptab")
  if ptab:count() == 0 then
    local owner = os.getenv("CHLOROBOT_OWNER")
    if owner ~= nil then
      local cloak = string.lower(owner)
      chlorobot.log("permissions CLEARING into owner ".. cloak)
      permissions.db:create("ptab", { cloak = {"unique"}, perms = ""})
      permissions.db:insert("ptab", { cloak = cloak, perms = "_ALL" })
      chlorobot.log("permissions SUCCESSFULLY CLEARED into owner ".. cloak)
      permissions.db:close()
    else
      permissions.db:close()
      error("please set environment variable CHLOROBOT_OWNER to your own IRC cloak")
    end
  end
end
