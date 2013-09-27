#!/usr/bin/ruby
#
# A little test to see if RequestReply works correctly

require "../../xmplib.rb"
error=""
begin
	xmpc=XmpConnector.new('localhost',30000)
	xmpc.Register("RequestReply")
rescue 
	puts "Unable to connect to the server.\nProbably the server isnt running"	
	exit
end

5.times do |x|
	xmpc.Request("replyserver","hallo #{x}")
	if xmpc.GetData!="hallo #{x}"
		error+="The received data is not equal to the send data"
	end
end

if error!=""
	puts error 
else
	puts "Everthing ok"
end
