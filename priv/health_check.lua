local http = require('socket.http')

health_check = {}

function health_check.heartbeat()
  local uri = os.getenv("CHLOROBOT_HEALTHCHECK_URI")

  if uri ~= nil then
    local body, code, head = http.request(uri)
    chlorobot.log('Sending health check heartbeat to specified server returned HTTP ' .. tostring(code))
  end
end

