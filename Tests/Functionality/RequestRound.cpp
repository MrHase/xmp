#include "RequestRound.h"

#include <iostream>
#include <pthread.h>
void* client1(void* ptr){
	try{
		xmp::XmpConnector connector("localhost",30000);
		connector.ServerReply(false);//we dont want replys from the server!
		connector.Register("client1");
		sleep(1);
		connector.Request("client2","token from 1");
		std::cout<<"c1: we got an answer...\n\t"<<connector.GetData()<<std::endl;
		sleep(10);
	}catch(xmp::Exception e){
		std::cout<<"Error! Is the server running?"<<std::endl;
	}
}
void* client2(void* ptr){
	try{
		xmp::XmpConnector connector("localhost",30000);
		connector.ServerReply(false);//we dont want replys from the server!
		connector.Register("client2");
		sleep(2);
		connector.Receive();
		std::cout<<"c2: receive...\n\t"<<connector.GetData()<<std::endl;
		connector.Reply(connector.GetSender(),connector.GetMsgID(),"token from 2");
		sleep(10);
	}catch(xmp::Exception e){
		std::cout<<"Error! Is the server running?"<<std::endl;
	}
}
void* client3(void* ptr){
	try{
		xmp::XmpConnector connector("localhost",30000);
		connector.ServerReply(false);//we dont want replys from the server!
		connector.Register("client3");
		sleep(3);
		connector.Receive();
		
		sleep(10);
	}catch(xmp::Exception e){
		std::cout<<"Error! Is the server running?"<<std::endl;
	}
}
void* spammer(void* ptr){
	try{
		xmp::XmpConnector connector("localhost",30000);
		connector.ServerReply(false);//we dont want replys from the server!
		connector.Register("spammer");
		while(true){
			connector.Send("client1","spam");
			sleep(1);
		}
		sleep(10);
	}catch(xmp::Exception e){
		std::cout<<"Error! Is the server running?"<<std::endl;
	}
}
int main(){
	int threadid=0;
	pthread_t thread;
	threadid=pthread_create(&thread,NULL,client1,(void*)0);
	threadid=pthread_create(&thread,NULL,client2,(void*)0);
	//threadid=pthread_create(&thread,NULL,client3,(void*)0);
	//threadid=pthread_create(&thread,NULL,spammer,(void*)0);
	while(true){
		
	}	
	return 0;	
}
