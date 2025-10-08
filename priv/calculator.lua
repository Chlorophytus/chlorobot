calculator = {
  stack_size_maximum = 12,
  operations = {
    ["+"] = function(lhs, rhs) return lhs + rhs end,
    ["-"] = function(lhs, rhs) return lhs - rhs end,
    ["*"] = function(lhs, rhs) return lhs * rhs end,
    ["/"] = function(lhs, rhs) return lhs / rhs end,
    ["%"] = function(lhs, rhs) return lhs % rhs end,
    ["^"] = function(lhs, rhs) return math.pow(lhs, rhs) end,
  },
}

function calculator.calculate(tokens)
  local stack = {}

  for token in tokens do
    if #stack > calculator.stack_size_maximum then
      return false, "too many operations"
    end

    local operation = calculator.operations[token]

    if operation ~= nil then
      if #stack < 2 then
        return false, "syntax error"
      end

      local rhs = table.remove(stack)
      local lhs = table.remove(stack)

      table.insert(stack, operation(lhs, rhs))
    else
      local num = tonumber(token)

      if num ~= nil then
        table.insert(stack, num)
      else
        return false, "invalid token"
      end
    end
  end

  return true, stack[#stack]
end