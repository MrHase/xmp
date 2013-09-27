#!/usr/bin/ruby

# 
# This simple ruby example sends
# 10 messages to "receiver".
# If "receiver" isnt registert,
# we get an error.
#

require "../../xmplib.rb"
begin
	xmpc=XmpConnector.new('localhost',30000)
	xmpc.Register("Example","rubyclient")
rescue 
	puts "Unable to connect to the server.\nProbably the server isnt running"	
	exit
end
begin
	
	10.times do |i|
		xmpc.Send("replyserver","a littel message<#{i}>")
	end
	puts "I have sent 10 messages to the replyserver\""
	10.times do
		xmpc.Receive
		puts xmpc.GetData
	end
	
	1.times do |i|
		doc = Document.new '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><testdata></testdata>'
		root=doc.root
		header=Element.new "testheader"
		type=Element.new "testtype"
		type.text="count"
		header.add_element type	
		root.add_element header
		xmpc.SendXML("replyserver",root)
	end
	puts "I have sent 1 xml messages to the replyserver\""
	1.times do
		xmpc.Receive
		puts xmpc.GetXML
	end
	
rescue RuntimeError => e
	puts "Something went wrong..."
	puts "Probably the receiverclient doesnt exist."
	puts "Errormessage: "
	puts e
	exit
end

