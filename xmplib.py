import socket
import sys
import xml.dom.minidom #xml
from struct import * #unpack, pack


class XmpConnector:
	
	def __init__(self,serveraddress,port):
		self.randomcounter=0
		self.name=''
		self.lastmessage=''
		
		## The socket creating looks a
		## little bit complicated, because
		## we support IPv4 and IPv6!
		HOST = serveraddress    # The remote host
		PORT = port # The same port as used by the server
		
		self.s = None
		for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC, socket.SOCK_STREAM):
			af, socktype, proto, canonname, sa = res
			try:
				self.s = socket.socket(af, socktype, proto)
			except socket.error, msg:
				self.s = None
				continue
			try:
				self.s.connect(sa)
			except socket.error, msg:
				self.s.close()
				self.s = None
				continue
			break
		if self.s is None:
			raise Exception('could not open socket\nprobably the server is not running')
			

	def send(self,msg):
		self.s.send('\0\0\0\0'+msg+'\0')
		
	def receive(self):
		data=''
		#print 'go'
		length=self.s.recv(4)
		#print 'go.'
		if length=='\0\0\0\0':
			#print "length 0000"
			#too slow .(
			while True:
				byte=self.s.recv(1)
				data<<byte
			#	#puts byte
				if byte=='\0':
					break;
			data.delete("\0") #remove the 0

		else:
			#print 'fast mode'
			#print ''.join([hex(ord(x))[2:] for x in length])#print length in hex
			#print [hex(ord(x))[2:] for x in length]#hex arrry
			
			length_array=[ord(x) for x in length]#dec array
			#print length_array
			
			bytestorecv=0
			counter=3
			for i in range(0,4):
				c=length_array[i]
				#print c
				bytestorecv+=(c*(2**(counter*8)))
				counter-=1
			
			#print 'bytes to receive: '
			#print bytestorecv
			while True:
				#print 'bytestorecv: '+str(bytestorecv)
				tmpdata=self.s.recv(bytestorecv)
				data+=tmpdata
				bytestorecv-=len(tmpdata)
				if bytestorecv==0:
					break;
				 
		return data
		
	def GetRandomString(self):
		self.randomcounter+=1
		if self.randomcounter==10:
			self.randomcounter=0
		stringarray=\
			'--Replacement--',\
			'++Replacement++',\
			'||Replacement||',\
			'((Replacement))',\
			'##Replacement##',\
			'91qaw5tzhnerf0oikj',\
			'i129lp,cjas,da,.',\
			'9mm,,x..spwklasfkefncxiuernwskaeuf78ewms',\
			'nwieu38r0ckemkjsdi',\
			'9ojyn.dfjiowfa',\
			'ioqe0123456789';
		return stringarray[self.randomcounter]
		
	def AddCleanMessage(self,msg,doc):
		newmsg=msg
		openstring=''
		closestring=''
		while True:
			openstring=self.GetRandomString()
			if newmsg.find(openstring)==-1: #till we found a string which is not contained in the msg
				break;
		while True:
			closestring=self.GetRandomString()
			if newmsg.find(closestring)==-1 and closestring!=openstring:
				break;
		
		
		newmsg=newmsg.replace('<',openstring)
		newmsg=newmsg.replace('>',closestring)
				
		# newmsg
		msg_elem = doc.createElement('text')
		msg_text = doc.createTextNode(newmsg)
		msg_elem.appendChild(msg_text)
		msg_elem.setAttribute("open-replacement", openstring)
		msg_elem.setAttribute("close-replacement", closestring)
		doc.documentElement.appendChild(msg_elem)
		
		
		#open_elem = doc.createElement('ReplaceOpen')
		#open_text = doc.createTextNode(openstring)
		#open_elem.appendChild(open_text)
		#doc.documentElement.appendChild(open_elem)	
		
		#close_elem = doc.createElement('ReplaceClose')
		#close_text = doc.createTextNode(closestring)
		#close_elem.appendChild(close_text)
		#doc.documentElement.appendChild(close_elem)
		
		
	def Register(self,appname,name):
		self.name=name
		import xml.dom
		# Dokument erzeugen
		implement = xml.dom.getDOMImplementation()
		doc = implement.createDocument(None, "msg", None)
		header=doc.createElement("header");
		elem = doc.createElement("type")
		elem.setAttribute("reply", "yes")
		text = doc.createTextNode("register")
		elem.appendChild(text)
		
		
		sender=doc.createElement("sender")
		sendertext=doc.createTextNode(name)
		sender.appendChild(sendertext)
		header.appendChild(sender)
		
		app=doc.createElement("application")
		apptext=doc.createTextNode(appname)
		app.appendChild(apptext)
		header.appendChild(app)
		
		header.appendChild(elem)
		doc.documentElement.appendChild(header)
		
		self.send(doc.toxml("utf-8"))
		
		#print doc.toprettyxml("utf-8")
		#print doc.toxml("utf-8")
		reply=self.receive()
		if reply.find('<error>')!=-1:		
			raise Exception(reply)
		return reply
	
	def Send(self,recv,msg):
		implement = xml.dom.getDOMImplementation()
		doc = implement.createDocument(None, "msg", None)
		
		header=doc.createElement("header")
		elem = doc.createElement("type")
		elem.setAttribute("reply", "yes")
		text = doc.createTextNode("normal")
		elem.appendChild(text)
		
		recv_elem = doc.createElement("receiver")
		recv_text = doc.createTextNode(recv)
		recv_elem.appendChild(recv_text)
		
		header.appendChild(elem)
		header.appendChild(recv_elem)
		doc.documentElement.appendChild(header)
		
		
		self.AddCleanMessage(msg,doc)
		self.send(doc.toxml("utf-8"))
		#print msg
		reply=self.receive()
		if reply.find('<error>')!=-1:		
			raise Exception(reply)
		return reply
	def SendXML(self,recv,xmldata):
		implement = xml.dom.getDOMImplementation()
		doc = implement.createDocument(None, "msg", None)
		
		header=doc.createElement("header")
		elem = doc.createElement("type")
		elem.setAttribute("reply", "yes")
		text = doc.createTextNode("normal")
		elem.appendChild(text)
		
		recv_elem = doc.createElement("receiver")
		recv_text = doc.createTextNode(recv)
		recv_elem.appendChild(recv_text)
		
		header.appendChild(elem)
		header.appendChild(recv_elem)
		doc.documentElement.appendChild(header)
		
		data=doc.createElement("data")
		data.appendChild(xmldata)
		doc.documentElement.appendChild(data)
		
		self.send(doc.toxml("utf-8"))
		#print msg
		reply=self.receive()
		if reply.find('<error>')!=-1:		
			raise Exception(reply)
		return reply
		
	def Request(self,recv,msg):
		#untested
		implement = xml.dom.getDOMImplementation()
		doc = implement.createDocument(None, "msg", None)
		
		header=doc.createElement("header")
		
		elem = doc.createElement("type")
		elem.setAttribute("reply", "yes")
		text = doc.createTextNode("request")
		elem.appendChild(text)
		
		recv_elem = doc.createElement("receiver")
		recv_text = doc.createTextNode(recv)
		recv_elem.appendChild(recv_text)
		
		header.appendChild(elem)
		header.appendChild(recv_elem)
		
		doc.documentElement.appendChild(header)
				
		self.AddCleanMessage(msg,doc)
		self.send(doc.toxml("utf-8"))
		#print msg
		reply=self.receive() ##server reply
		if reply.find('<error>')!=-1:		
			raise Exception(reply)
		reply=self.receive() #client reply
		if reply.find('<error>')!=-1:
			raise Exception(reply)
		self.lastmessage=reply
		
	def Reply(self,recv,msgid,msg):
		implement = xml.dom.getDOMImplementation()
		doc = implement.createDocument(None, "msg", None)
		
		header=doc.createElement("header")
		
		elem = doc.createElement("type")
		elem.setAttribute("reply", "yes")
		text = doc.createTextNode("reply")
		elem.appendChild(text)
		
		recv_elem = doc.createElement("receiver")
		recv_text = doc.createTextNode(recv)
		recv_elem.appendChild(recv_text)
		
		id_elem=doc.createElement("ReplyID")
		id_text = doc.createTextNode(msgid)
		id_elem.appendChild(id_text)
		
		
		header.appendChild(elem)
		header.appendChild(recv_elem)
		header.appendChild(id_elem)
		
		doc.documentElement.appendChild(header)
		
		self.AddCleanMessage(msg,doc)
		#print 'reply : '+doc.toxml("utf-8")
		self.send(doc.toxml("utf-8"))
		
		reply=self.receive()
		if reply.find('<error>')!=-1:		
			raise Exception(reply)
		return reply
		
	def Count(self):
		implement = xml.dom.getDOMImplementation()
		doc = implement.createDocument(None, "msg", None)
		
		header = doc.createElement("header")
		
		elem = doc.createElement("type")
		elem.setAttribute("reply", "yes")
		text = doc.createTextNode("count")
		elem.appendChild(text)
		
		header.appendChild(elem)
		doc.documentElement.appendChild(header)
				
		self.send(doc.toxml("utf-8"))
				
		reply=self.receive()
		if reply.find('<error>')!=-1:		
			raise Exception(reply)
		data=''
		
		try:
			doc_reply = xml.dom.minidom.parseString(reply)
			#data=doc_reply.toxml("utf-8")
			#print doc_reply.toxml("utf-8")
			nr = doc_reply.getElementsByTagName("text")[0]
			#print nr.toxml("utf-8")
			#print nr.firstChild.data
			return nr.firstChild.data #(nr=<text>0</text>)
		except(TypeError,AttributeError):
			raise Exception(reply)
		
			

	def Receive(self):
		implement = xml.dom.getDOMImplementation()
		doc = implement.createDocument(None, "msg", None)
		
		header = doc.createElement("header")
		
		elem = doc.createElement("type")
		elem.setAttribute("reply", "yes")
		text = doc.createTextNode("receive")
		elem.appendChild(text)
		
		header.appendChild(elem)
		doc.documentElement.appendChild(header)
				
		self.send(doc.toxml("utf-8"))
				
		reply=self.receive()     ##serverreply
		if reply.find('<error>')!=-1:		
			raise Exception(reply)
		recvdata=self.receive() ##receive
		self.lastmessage=recvdata
				
	def GetMsgID(self):
		try:
			doc_reply = xml.dom.minidom.parseString(self.lastmessage)
			#print doc_reply.toxml("utf-8")
			msgid = doc_reply.getElementsByTagName("MsgID")[0]
			return msgid.firstChild.data 
		except StandardError,e:
			raise Exception(e)	
	def GetType(self):
		try:
			doc_reply = xml.dom.minidom.parseString(self.lastmessage)
			#print doc_reply.toxml("utf-8")
			header= doc_reply.getElementsByTagName("header")[0]
			msgid = header.getElementsByTagName("type")[0]
			return msgid.firstChild.data 
		except StandardError,e:
			raise Exception(e)	
	def GetSender(self):
		try:
			doc_reply = xml.dom.minidom.parseString(self.lastmessage)
			sender = doc_reply.getElementsByTagName("sender")[0]
			return sender.firstChild.data 
		except StandardError,e:
			raise Exception(e)	
	def GetMsg(self):
		return self.lastmessage
	def GetData(self):
		try:
			#print 'lastmessage:'+self.lastmessage
			doc_reply = xml.dom.minidom.parseString(self.lastmessage)
			rawdata = doc_reply.getElementsByTagName("text")[0]
			data=''
			if rawdata!='' and rawdata!=None:
				text=rawdata.firstChild.data 
				
				if rawdata.attributes!=None and rawdata.attributes.length!=0:
					open=rawdata.attributes["open-replacement"] #! untestetd!!!
					if open!=None and text.find(open.value)!=-1:
						text=text.replace(open.value,'<')
					close=rawdata.attributes["close-replacement"]
					if close!=None and text.find(close.value)!=-1:
						text=text.replace(close.value,'>')
					
			return text
		except StandardError,e:
			raise Exception(e)	
	def GetXML(self):
		try:
			#print 'lastmessage:'+self.lastmessage
			doc_reply = xml.dom.minidom.parseString(self.lastmessage)
			data=None
			if doc_reply.getElementsByTagName("data")!=None:
				if doc_reply.getElementsByTagName("data").length>0:
					data = doc_reply.getElementsByTagName("data")[0]
			if data!=None:
				return data.firstChild
			return None
			
		except StandardError,e:
			raise Exception(e)	
		
		
	
