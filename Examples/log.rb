require "../xmplib.rb"

xmpc=XmpConnector.new("localhost",30000)
xmpc.Register("Example","ManagerLog") ## This name is reserved for the logger

while true do
	puts "Logs: #{xmpc.Count}"
	xmpc.Receive
	puts ""
	puts xmpc.GetMsg
	puts ""
end 
