import sys
import time
sys.path.append("./../../") #We add the path to the libs here..
import xmplib

try:
	s=xmplib.XmpConnector('localhost',30000)
except (Exception):
	print('Error:\nUnable to connect to the server.'+\
		'Start the server before you run this example\n'+\
		'Take a look in the doc how to compile and run the server\n')
	sys.exit(1)

s.Register('test')

s.Send('test','<hello>xxx</hello>')
s.Receive()

print(s.GetData())
