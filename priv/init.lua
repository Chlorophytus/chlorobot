local joining = true

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
  end
end
