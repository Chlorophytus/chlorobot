local sqlite = require('sqlite')

permissions = { db = sqlite{ uri = "tables/ptab.db" } }

function permissions.is_legal(perm)
  local legality = 0
  local check_char = function(checking)
    -- Lua purges ok_chars every time so we have to respecify it
    local ok_chars = string.gmatch("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ".")

    -- check for legal chars one by one, Lua doesn't give us a frozenset type
    for checker in ok_chars do
      if checker == checking then return true end
    end

    -- illegal char, so return
    return false
  end

  -- loop through all chars and give a count of how many legal chars there are
  for i = 1, #perm do
    if check_char(perm[i]) then
      legality = legality + 1
    end
  end

  -- returns true if legal char count is equal to the permission's char count
  return legality == #perm
end

function permissions.bulk_get(raw_cloak)
  local cloak = string.lower(raw_cloak)
  permissions.db:open()
  local entry = permissions.db:select("ptab", { where = { cloak = cloak } })
  permissions.db:close()
  print("permissions bulk_get (" .. cloak .. ") -> " .. entry)
  local split = {}

  for perm in string.gmatch(entry.perms, "([^,]+)") do
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
        print("permissions get (" .. cloak .. "): " .. perm .. " -> has permission")
        return true
      elseif this_perm == "_ALL" then
        print("permissions get (" .. cloak .. "): " .. perm .. " -> has _ALL")
        return true
      end
    end

    print("permissions get (" .. cloak .. "): " .. perm .. " -> does not have permission")
    return false
  else
    print("permissions get (" .. cloak .. "): " .. perm .. " -> illegal permission specified")
    return false
  end
end

function permissions.put(raw_cloak, perm, state)
  local cloak = string.lower(raw_cloak)
  if perm[1] ~= "_" and permissions.is_legal(perm) then
    local bulk = permissions.bulk_get(cloak)
    local perm_at = nil

    for i = 1, #bulk do
      local this_perm = bulk[i]
      if this_perm[1] == "_" then
        print("permissions put (" .. cloak .. "): " .. perm .. " -> user has '_' sticky permission prefix")
        return
      elseif this_perm == perm then
        if state then
          print("permissions put (" .. cloak .. "): " .. perm .. " SET -> user already has permission")
          return
        elseif perm_at == nil then
          perm_at = i
        end
      end
    end

    if state then
      table.insert(bulk, perm)
      print("permissions put (" .. cloak .. "): SET" .. perm .. " -> OK")
    elseif perm_at ~= nil then
      table.remove(bulk, perm_at)
      print("permissions put (" .. cloak .. "): CLEAR" .. perm .. " -> OK")
    else
      print("permissions put (" .. cloak .. "): CLEAR" .. perm .. " -> user doesn't have permission")
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
    print("permissions put (" .. cloak .. "): " .. perm .. " -> illegal or sticky permission specified")
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
      print("permissions CLEARING into owner ".. cloak)
      permissions.db:create("ptab", { cloak = {"unique"}, perms = ""})
      permissions.db:insert("ptab", { cloak = cloak, perms = "_ALL" })
      print("permissions SUCCESSFULLY CLEARED into owner ".. cloak)
      permissions.db:close()
    else
      permissions.db:close()
      error("please set environment variable CHLOROBOT_OWNER to your own IRC cloak")
    end
  end
end
