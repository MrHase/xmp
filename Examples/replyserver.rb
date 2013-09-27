#!/usr/bin/ruby
#
#This little programm return the data to the sender

require "../xmplib.rb"

begin
	xmpc=XmpConnector.new('localhost',30000)
	xmpc.Register("Example","replyserver")
rescue 
	puts "Unable to connect to the server.\nProbably the server isnt running"	
	exit
end

while true
	begin
		xmpc.Receive
		sender=xmpc.GetSender
		data=xmpc.GetData
		type=xmpc.GetType
		
		if data!=nil and type!=nil 
			if type=="normal"
				puts "normal data: "+data.to_s
				xmpc.Send(sender,data) 
			end
			if type=="request"
				xmpc.SendReply(sender,xmpc.GetMsgID,data)
			end
		else
			puts "keine Daten enthalten!"
			puts "msg:"
			puts xmpc.GetMsg()
		end
	rescue RuntimeError=>e
		puts "We get an error: "+e
	end
end
