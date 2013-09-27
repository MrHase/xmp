import sys
sys.path.append("./../../") #We add the path to the libs here..
import xmplib
import xml.dom.minidom #xml

try:
	s=xmplib.XmpConnector('localhost',30000)
except (Exception):
	print('Error:\nUnable to connect to the server.'+\
		'Start the server before you run this example\n'+\
		'Take a look in the doc how to compile and run the server\n')
	sys.exit(1)

s.Register('Example','test')
#we send some messages to ourself!
s.Send('test','hello')
s.Send('test','this message is easy to handle, but what about the next one!?!?!')
s.Send('test','<hallo>zzz</hallo>this is a testmessage<<++Replacement++>>')


#we count the messages for us on the server:
count=s.Count()
print 'Count: '+count
#and receive them:
for i in range(0,int(count)):
	s.Receive()
	#Mostly we just need the Data... :
	print 'Msg ID: '+s.GetMsgID()
	print 'Sender: '+s.GetSender()
	#print 'RawMsg: '+s.GetMsg()
	print 'Data: '+ s.GetData()

#now were sending xml:
implement = xml.dom.getDOMImplementation()
doc = implement.createDocument(None, "testdata", None)
test=doc.createElement("data")
text = doc.createTextNode("normal")
test.appendChild(text)
doc.documentElement.appendChild(test)

print 'XmlData: '+test.toxml("utf8")
s.SendXML('test',test)
s.Receive()
print 'XmlData: '+s.GetXML().toxml("utf8")
