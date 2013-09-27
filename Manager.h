//! since xml is buggi we must delete the
//! added Elements ourself!!!(in AddElement...)


#ifndef MANAGER_H
#define MANAGER_H

//the number of messages every client can save
#define MAX_CLIENT_QUEUE_SIZE 500 


#define TIXML_USE_STL
#define TIXML_USE_TICPP

#include <iostream>
#include <map>
#include <queue>
#include <deque>
#include <list>

#include "Socket.h"
#include "SocketException.h"
#include <pthread.h>
#include "./tinyxml++/ticpp.h"
#include "ThreadsafeQueue.h"
#include "src/Log.h"

#define LOGNAME "ManagerLog"


class ManagerException{
	public:
	ManagerException(std::string error_){
		error=error_;
	}
	std::string GetError(){
		return error;	
	}
	private:
	std::string error;
};

class Client{
	public:
		Client(std::string name_,int threadid_,Socket* socket_){
			threadid=threadid_;
			socket=socket_;
			name=name_;
			waiting_for_reply=0;
			waiting_for_message=false;
			//std::cout <<"Client erstellt: name: "<<name<<" threadid: "<<threadid<<" Socket: "<<socket<<std::endl;
		}
		void SetReplyID(unsigned int id){
			replyid=id;
		}
		unsigned int GetReplyID(){
			return replyid;	
		}
		void WaitingForMessage(bool waiting){
			waiting_for_message=waiting;
		}
		bool WaitingForMessage(){
			return waiting_for_message;
		}
		void WaitingForReply(std::string name){
			//! we should save the name so we can see which reply is missing
			//std::cout <<" we wait for a reply from "<<name<<std::endl;
			waiting_for_reply++;
		}
		int WaitingForReply(){
			return waiting_for_reply;
		}
		void GetReplyFrom(std::string msg,std::string name){
			//! remove the name from the replylist
			if(waiting_for_reply==0);//std::cout<<"Error: Wir warten auf kein Reply";
			else {
				if(	GetSocket()->send(msg) ==false){ //direct send
					throw ManagerException("Client - GetReplyFrom: Error while Hardwaresend");
				}				
				waiting_for_reply--;	
			}
			
			//std::cout <<" we get a reply from "<<name<<std::endl;
		}

		int Send(const std::string msg){
			if(WaitingForMessage() && waiting_for_reply==0){//if we wait for replys disabel direct send
				//the Client is already waiting for messages so we can send it directly
				if(	GetSocket()->send(msg) ==false){ //direct send
					throw ManagerException("Client - Send: Error while Hardwaresend");
				}				
				WaitingForMessage(false);
			}else{
				//if we have too much messages in the queue throw error
				if(GetReceiveQueue()->size()<MAX_CLIENT_QUEUE_SIZE){
					GetReceiveQueue()->push_back(msg);
				}else{
					throw ManagerException("Client - Send: Too much messages in the queue");	
				}
			}
			return 0;
		}
		//wenn eine sendung mit reply losgeschickt wurde, wird das programm ja solange blockieren, bis
		//alle reply(direct) angekommen sind. Daher muss im Receive sich gar nciht darum gekümmert werden
		int Receive(){
			if(GetReceiveQueue()->empty()==true){
				//std::cout<<"Client: Rec P3\n";
				WaitingForMessage(true);
			}else{
				//std::cout<<"Client: Rec P4\n";
				std::string msg =GetReceiveQueue()->front();
				GetReceiveQueue()->pop_front();
				if(	GetSocket()->send(msg) ==false){
					throw ManagerException("Client - Receive: Error while Hardwarereceive");
				}
			}
			return 0;			
		}
		std::deque<std::string>* GetReceiveQueue(){
			return &receivequeue;	
		}
		Socket* GetSocket(){return socket;}
		int GetThreadID(){return threadid;}
		std::string GetName(){return name;}
	private:
		std::deque<std::string> receivequeue;
		int threadid;
		Socket* socket;
		std::string name;
		int waiting_for_reply;
		unsigned int replyid;
		bool waiting_for_message;
		
};

class Manager{
public:
	Manager(int port);
	int run();
	//--------------------------------
	//Visible for the threads
	int Incomming(const int id, const std::string& msg);
	int RegisterSocket(const int id, Socket* socket);
	//--------------------------------
private:

	//--------------------------------
	//Handle the message
	int Error(const std::string& msg,const bool sendreply);
	int Register(const std::string& msg,const bool sendreply);
	int Send(const std::string& msg,const bool sendreply);
	int Request(const std::string& msg,const bool sendreply);
	int Receive(const std::string& msg,const bool sendreply);
	int Count(const std::string& msg,const bool sendreply);
	int Reply(const std::string& msg,const bool sendreply);
	int Disconnect(const std::string& msg,const bool sendreply);
	//--------------------------------
	
	void CheckMessage(const std::string& msg);
	int SendReply(const std::string& msg);
	int SendToClient(const std::string& name,const std::string& msg);
	int AddError(std::string& msg,const std::string& error);
	int Trash(const std::string msg);
	int Log(const std::string& msg);
	Client* GetClient(const std::string& app,const std::string& name);
	std::map<std::string,Client*>* GetClientMap(const std::string& app);
	void AddApplication(const std::string& app);
	void RemoveApplication(const std::string& app);
	unsigned int IDgenerator();
	bool ElementExist(const ticpp::Element* pElem,const std::string& name)const;
	//int AddElement( ticpp::Element* pElem,const std::string& name,const std::string& text);
	bool ClientRegistered(const int id);
	bool ClientRegistered(const ticpp::Element* const pElem);
	std::string GetName(const int id);
	std::string GetApp(const int id);
	std::string GetSender(const int id);
	template<class T>
	int AddElement( ticpp::Element* pElem,const std::string& name,const T& text){
		if(pElem!=NULL){
			//!-------buggi xml
			pthread_mutex_lock(&buggixmlmutex);
			if(buggixmlelement.size()>9){
				delete (buggixmlelement.front());
				buggixmlelement.pop();
				
			}
			if(buggixmltext.size()>9){
				delete (buggixmltext.front());
				buggixmltext.pop();
			}
			//!--------------
			ticpp::Element* element=new ticpp::Element(name);
			ticpp::Text* elementtext=new ticpp::Text(text);
			element->LinkEndChild(elementtext);
			pElem->LinkEndChild(element);
			
			//!-------buggi xml
			buggixmlelement.push(element);
			buggixmltext.push(elementtext);
			pthread_mutex_unlock(&buggixmlmutex);
			//!--------------
		}else throw ticpp::Exception("AddElement: pElem==NULL \n");
		return 0;
		
	}
	unsigned int GetMsgID(const ticpp::Element* const pElem)const;
	unsigned int GetReplyID(const ticpp::Element* const pElem)const;
	std::string GetSender(const ticpp::Element* pElem);
	std::string GetReceiver(const ticpp::Element* pElem);
	std::string GetApp(const ticpp::Element* pElem);
	ticpp::Element* FindElement(const ticpp::Element* pElem,const std::string& name)const;
	std::map<int,Socket*>* GetSocketMap();
	
	
	//! just because xml is buggi!
	std::queue<ticpp::Element*> buggixmlelement;
	std::queue<ticpp::Text*> buggixmltext;	
	pthread_mutex_t buggixmlmutex;
	//!-------------	
	Socket socket;
	std::map<int,Socket*> socketmap;
	int port_;
	unsigned int id_;
	StringQueue incomming;
	std::list<std::string> trash;
	std::list<std::string> log;
	//std::map<std::string,Client*>clientmap;
	std::map<std::string,std::map<std::string,Client*>* >clientmap; //map<application,map<clientname,data>>
	
};


class threadparameter{
	public:
	Socket* socket;	
	Manager* manager;
	int nr;
	int port;
	
};

void* serverthread(void* ptr){
	Socket* server;
	threadparameter* para;
	para=(threadparameter*)ptr;
	server=para->socket;
		
	try
	{
		int status;
		bool error=false;
		while(!error)
		{
			std::string data;
			status=server->recv(data);
			if(status<=0){
				//std::cout <<" Es ist ein Fehler beim empfangen aufgetreten"<<std::endl;
				error=true;
			}else{
				para->manager->Incomming(para->nr,data);
				//std::cout<<"data: "<<data<<std::endl;
			}		
		}
	}catch ( SocketException& e) {
		//std::cout << "Exception was caught:" << e.description() << "\nExiting.\n";	
	}

	std::string dc="<msg><header><type>disconnect</type></header><reason>connection crashed</reason></msg>";
	para->manager->Incomming(para->nr,dc);
	
	delete para->socket;// socket close in the Destructor
	delete para;

	//std::cout << "thread "<<para->nr<<" beendet"<<std::endl;
	return NULL;//avoid compilerwarnings..
}
void* server(void* ptr){
	threadparameter* parameter;
	parameter=(threadparameter*)ptr;
	Socket socket;
	int port_=parameter->port;
	if ( ! socket.create() ) {
		throw SocketException ( "Could not create server socket." );
	}
	if ( ! socket.bind ( port_ ) ) {
		throw SocketException ( "Could not bind to port." );
	}
	if ( ! socket.listen() ) {
		throw SocketException ( "Could not listen to socket." );
	}
	unsigned int i=0;
	while (1) {
		pthread_t thread;
		int threadid;
		threadparameter* para=new threadparameter;
		
		Socket* new_sock= new Socket();
		socket.accept ( *new_sock );
		para->socket=new_sock;
		para->nr=i;
		para->manager=parameter->manager;
		parameter->manager->RegisterSocket(i,new_sock);
		i++;
		
		threadid=pthread_create(&thread,NULL,serverthread,(void*)para);
	}
}


#endif

