require "rexml/document"
require 'socket'

include REXML

class XmpConnector

	def initialize(serveraddr,port)
		@ConnectionName=""
		@ApplicationName=""
		socket=TCPSocket.new(serveraddr,port)
		@clientSocket=socket
		@lastmessage=""
		@randomstringcounter=0	
		@recvbuffer=""
	end
	
	def send(socket,data)
		#! viel zu langsam!
		socket.write("\0\0\0\0#{data}\0")
		#puts "senden länge: #{data.length()+1}"
	end

	def receive(socket)
		#puts "receive"
		data=""
		length=socket.recv(4) 
		if length=="\0\0\0\0" then
			#puts "length 0000"
			#too slow .(
			begin
				byte=socket.recv(1)
				data<<byte
			#	#puts byte
			end while byte!="\0"
			data.delete("\0") #remove the 0

		else
			#untestet
			#get length bytes
			bytestorecv=0
			counter=3
			length.each_byte do |c|
				#puts "byte: #{c}"
				bytestorecv+=(c*(2**(counter*8)))
				counter-=1
			end
			#puts "bytes to recv: #{bytestorecv}"
			begin
				#puts "      bytestorecv: #{bytestorecv}"
				tmpdata=socket.recv(bytestorecv)
				data+=tmpdata
				bytestorecv-=tmpdata.length
			end while bytestorecv!=0
						
		end 
		#puts "empfangene daten: #{data}"
		#puts "datenlänge: #{data.length}"
		return data
	end
	def Register(app,name)
		@ConnectionName=name
		@ApplicationName=app
		doc = Document.new '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><msg></msg>'
		root=doc.root
		header=Element.new "header"
		type=Element.new "type"
			type.attributes["reply"]="yes"
			#puts type.type
			type.text="register"
		sender=Element.new "sender"
			sender.text="#{name}"
		app=Element.new "application"
			app.text=@ApplicationName
		
		header.add_element type
		header.add_element sender
		header.add_element app
		root.add_element header
		send(@clientSocket,root.to_s);
		#puts "Reply: #{receive(@clientSocket)}"
		reply=receive(@clientSocket)
		raise GetError(reply) if GetError(reply)!=""
	end

	def Send(rec,xml)
		
		#! hier lieber mal den typ von xml überprüfen!
		doc = Document.new '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><msg></msg>'
		root=doc.root
		header=Element.new "header"
		type=Element.new "type"
			type.attributes["reply"]="yes"
			type.text="normal"
		receiver=Element.new "receiver"
			receiver.text="#{rec}"
		
		header.add_element type
		header.add_element receiver
		root.add_element header
		begin
			xmldata=Element.new "data"
			xmldata.add_element xml
			root.add_element xmldata
			
		rescue
			puts "EXCEPTION"
			#! throw exception
			return
		end			
		
		send(@clientSocket,root.to_s);
		
		reply=receive(@clientSocket)
		raise GetError(reply) if GetError(reply)!=""
		return reply
	end
	
	def Request(rec,msg)#send und
		replaceOpen=""
		replaceClose=""
		msg,replaceOpen,replaceClose=ReplaceBadSymbols(msg)

	
		doc = Document.new '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><msg></msg>'
		root=doc.root
		header=Element.new "header"
		type=Element.new "type"
			type.attributes["reply"]="yes"
			type.text="request"
		receiver=Element.new "receiver"
			receiver.text="#{rec}"
		text=Element.new "text"
			text.text="#{msg}"
			text.attributes["open-replacement"]=replaceOpen if replaceOpen!=""
			text.attributes["close-replacement"]=replaceClose if replaceClose!=""
		header.add_element type
		header.add_element receiver
		root.add_element header
		root.add_element text
		
		
		send(@clientSocket,root.to_s);
		reply=receive(@clientSocket)#thats the serverreply
		raise GetError(reply) if GetError(reply)!=""
		reply=receive(@clientSocket)#thats the client reply
		raise GetError(reply) if GetError(reply)!=""
		@lastmessage=reply
		return GetData()
	end
	
	def Reply(rec,msgid,msg)

		replaceOpen=""
		replaceClose=""
		msg,replaceOpen,replaceClose=ReplaceBadSymbols(msg)
	
	
		doc = Document.new '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><msg></msg>'
		root=doc.root
		header=Element.new "header"
		type=Element.new "type"
			type.attributes["reply"]="yes"
			type.text="reply"
		receiver=Element.new "receiver"
			receiver.text="#{rec}"
		replyid=Element.new "ReplyID"
			replyid.text="#{msgid}"
		text=Element.new "text"
			text.text="#{msg}"
			text.attributes["open-replacement"]=replaceOpen if replaceOpen!=""
			text.attributes["close-replacement"]=replaceClose if replaceClose!=""
		
		
		header.add_element type
		header.add_element receiver
		header.add_element replyid
		root.add_element header
		root.add_element text
		

		
		send(@clientSocket,root.to_s);
		reply=receive(@clientSocket)#thats the serverreply
		raise GetError(reply) if GetError(reply)!=""
		return reply
	end
	
	def Count
		doc = Document.new '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><msg></msg>'
		root=doc.root
		header=Element.new "header"
		type=Element.new "type"
			type.attributes["reply"]="yes"
			type.text="count"
		header.add_element type	
		root.add_element header
		send(@clientSocket,root.to_s);
		reply=receive(@clientSocket)#thats the serverreply
		raise GetError(reply) if GetError(reply)!=""
		#puts "\n reply: \n #{reply} \n"
		data=""
		doc = Document.new reply
		root = doc.root
		if root.elements["text"]!=nil then
			data = root.elements["text"].text
		end
	
		return data
	end
	def Receive
		doc = Document.new '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><msg></msg>'
		root=doc.root
		header=Element.new "header"
		type=Element.new "type"
			type.attributes["reply"]="yes"
			type.text="receive"
		header.add_element type
		root.add_element header
		
		send(@clientSocket,root.to_s);
		reply=receive(@clientSocket)#thats the serverreply
		raise GetError(reply) if GetError(reply)!=""
		#puts "rec1"
		recvdata=receive(@clientSocket)
		#puts "rec2"
		
		@lastmessage=recvdata
		return GetData()
	end
	def GetType
		data=""
		#puts @lastmessage
		doc = Document.new @lastmessage
		root = doc.root
		header=root.elements["header"]
		data = header.elements["type"].text
		return data		
	end
	def GetSender
		data=""
		doc = Document.new @lastmessage
		root = doc.root
		data = root.elements["sender"].text
		return data		
	end
	def GetMsgID
		data=""
		doc = Document.new @lastmessage
		root = doc.root
		data = root.elements["MsgID"].text
		return data		
	end
	def GetMsg
		return @lastmessage
	end
	def GetData()
		doc = Document.new @lastmessage
		root = doc.root
		#root.elements.each{|e|
		#	puts e
		#}
		if root.elements["data"]!=nil then
			#return root.elements["data"]
			return root.elements["data"].elements[1]
		else
			return nil
		end
	end
	def GetError(msg)
		data=""
		doc = Document.new msg
		root = doc.root
		if root.elements["error"]!=nil
			data = root.elements["error"].text
		end
		return data		
	
	end
end
