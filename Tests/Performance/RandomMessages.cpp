#include <iostream>

#include "../../Socket.h"
#include "../../xmplib.hpp"
#include <cstring>

using namespace std;

int main(){
    const int buffersize=10000;
    char buffer[buffersize];
    cout<<"Filling Testbuffer"<<endl;
    memset(buffer,'a',buffersize);
    buffer[buffersize-1]='\0';
	Socket socket;
	if ( ! socket.create() )
	{
		cout<< "Could not create client socket."<<endl;
	}
	if ( ! socket.connect ( "127.0.0.1", 30000 ) )
	{
		cout<<"Could not bind to port."<<endl;
	}
	
    /*
    for(int i=1;i<20;i++){
        int s=i*100;
        cout<<"Sending size: "<<s<<endl;
        char testbuffer[s];
        memcpy(testbuffer,buffer,s);
        testbuffer[s-1]='\0';
        //printf("bla: %s\n",testbuffer);
        string teststring(testbuffer);
        socket.send(teststring,true);
        //cout<<"Size: "<<teststring.size()<<" Teststring: "<<teststring<<endl;

    }
    */

    try{
        xmp::XmpConnector test("127.0.0.1",30000);
        cout<<"sending stuff"<<endl;
        for(int i=1;i<20000000;i++){
            test.Send("hhoo","<hallo>test</hallo>");
        }


    }catch(xmp::Exception& e){
        cout<<"Exception: "<<e.GetError()<<endl;
    }

}
