#include "Manager.h"
#define TIXML_USE_STL
#define TIXML_USE_TICPP
#define STR(str) std::string(str)
Manager::Manager(int port){
	SetScope("Manager","Manager");
	
	port_=port;
	//buggixmlmutex= PTHREAD_MUTEX_INITIALIZER ;
	pthread_mutex_init(&buggixmlmutex,NULL);
	id_=0;
	pthread_t thread;
	int threadid;
	TiXmlBase::SetCondenseWhiteSpace(false); //this is very important! otherwise tinyxml replace tabs with whitespaces...
	threadparameter* para=new threadparameter;
	para->manager=this;
	para->port=port;
	LOG("starting serverthread");
	threadid=pthread_create(&thread,NULL,server,(void*)para);
}

int Manager::run(){
	SetScope("Manager","run");
	std::string msg;
	
	while(true){
		bool log=true;
		bool sendreply=false; 
		msg=incomming.pop();
		LOG(STR("incomming: ")+=msg);
		ticpp::Element* pElem;
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		pElem=doc.FirstChildElement(); //root

		try{
			if(!ElementExist(pElem,"header"))		throw ManagerException("Manager - run: header is missing");
			if(ElementExist(pElem,"application"))	throw ManagerException("Manager - run: Application already existing");
			if(ElementExist(pElem,"sender"))		throw ManagerException("Manager - run: Sender already existing");
			
			const ticpp::Element* const pElemHeader=FindElement(pElem,"header");
			if(!ElementExist(pElemHeader,"type"))	throw ManagerException("Manager - run: type is missing");
			
			if(ClientRegistered(pElem)){
				AddElement(pElem,"application",GetApp(pElem));
				AddElement(pElem,"sender",GetSender(pElem));	
				msg=doc.ToString(doc);
			}else{
				if((FindElement(pElemHeader,"type")->GetText().compare("register")!=0)){
					throw ManagerException("Manager - run: An unregistered message is not from type 'register'");
				}
			}
					
						
			pElem=FindElement(pElemHeader,"type");
			try{
				std::string attr;
				pElem->GetAttribute( "reply", &attr );
				if(attr.compare("yes")==0) sendreply=true;
			}catch(ticpp::Exception& ex ){/*std::cout <<"no attributes!!!!!!!!"<<std::endl;*no attributes is not an error...*/}
			
			if(pElem->GetText().compare("register")==0)		Register(msg,sendreply);
			if(pElem->GetText().compare("error")==0)		Error(msg,sendreply);
			//!if(pElem->GetText().compare("broadcast")==0)	Register(msg,sendreply);//wie handlen wir da ein reply von jedem teilnehmer? da der client ja nicht weiß wie viele replys er abholen muss. die anzahl müsste also im server reply stehen
			if(pElem->GetText().compare("count")==0)		Count(msg,sendreply);
			if(pElem->GetText().compare("normal")==0)		Send(msg,sendreply);
			if(pElem->GetText().compare("request")==0)		Request(msg,sendreply);
			if(pElem->GetText().compare("disconnect")==0)	Disconnect(msg,sendreply);//hier gibt es Probleme beim Loggen wenn der Client schon gelöscht ist, kann im Log nciht mehr die App rausgefunden werden... daher muss es vorher geloggt werden
			if(pElem->GetText().compare("reply")==0)		Reply(msg,sendreply);							
			if(pElem->GetText().compare("receive")==0)		Receive(msg,sendreply);
			if(log)Log(msg);			
		}catch(ticpp::Exception& ex ){
			std::string errormsg=msg;
			AddError(errormsg,"Manager - run: XmlError");
			Log(errormsg);
		}catch(ManagerException& me){
			std::string errormsg=msg;
			AddError(errormsg,me.GetError());
			Log(errormsg);
		}	
	}//while(true)

}

int Manager::RegisterSocket(const int id, Socket* socket){
	SetScope("Manager","RegisterSocket");
	//! Log this???
	(*GetSocketMap())[id]=socket;
	return 0;
}
int Manager::Incomming(const int id, const std::string& msg){
	SetScope("Manager","Incomming");
	//we add the threadid and the msg id here!
	//std::cout<<"id : "<<id<<" msg: "<<msg<<"\n";
	
	std::string newmsg;
	try{
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		ticpp::Element* pElem;
		pElem=doc.FirstChildElement(); //root
		
		if(!(pElem-> Value().compare("msg")==0)){		//the root elemen must be <msg></msg>
			throw ManagerException ("Manager - Error: the root element must be <msg>\n");						
		}
	
		if(ElementExist(pElem,"ThreadID"))throw ManagerException("Manager - Incomming: ThreadID already exist");
		if(ElementExist(pElem,"MsgID"))throw ManagerException("Manager - Incomming: MsgID already exist");
		AddElement(pElem,"ThreadID",id);			//add the ThreadID
		AddElement(pElem,"MsgID",IDgenerator());	//add the MsgID

		newmsg=doc.ToString(doc);
				
	}catch(ticpp::Exception& ex ){
		//std::cout<<"Manager - Error: Error in Incomming: " <<msg<<"\n";
		return -1;
	}catch(ManagerException& me){
		//std::cout<<"Manager - Error: Error in Incomming: "<<me.GetError()<<"\n";
		return -1;
	}	
	incomming.push(newmsg);
	return 0;
}

int Manager::Error(const std::string& msg,const bool sendreply){
	SetScope("Manager","Error");
	try{	
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		ticpp::Element* pElem=doc.FirstChildElement();
		ticpp::Element* pElem2=FindElement(pElem,"ThreadID");
		int threadid;
		pElem2->GetText(&threadid);
		pElem=FindElement(FindElement(pElem,"error"),"text");
		if(pElem->GetText().compare("connection crashed")==0){
			//std::cout <<"Error... Thread "<<threadid<<" crashed"<<std::endl;
			(*GetSocketMap()).erase(threadid);//erase the id+socket from the list
			if(!(GetName(threadid).empty())){ //find client from this thread and disconnect it
				std::string ID=pElem2->GetText();
				
				std::string dcmsg="<msg><type>disconnect</type><ThreadID>"+ID+"</ThreadID><sender>"+GetName(threadid)+"</sender><application>"+GetApp(threadid)+"</application></msg>";
				Disconnect(dcmsg,false);
			}			
		}
	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - Error: Error in Error");
	}	
	return 0;	
}
int Manager::Register(const std::string& msg,const bool sendreply){
	SetScope("Manager","Register");
	try{
		LOG(STR("daten: ")+=msg);
		
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		//if(!ElementExist(pElem,"ThreadID")) throw ManagerException("Manager - Register: No ThreadID found");
		//if(!ElementExist(pElem,"sender")) throw ManagerException("Manager - Register: No sender found");
		//if(!ElementExist(pElem,"application"))throw ManagerException("Manager - Register: No application found");
		const ticpp::Element* const pElemRoot=doc.FirstChildElement(); //root
		const ticpp::Element* const pElemID=FindElement(pElemRoot,"ThreadID");
		const ticpp::Element* const pElemHeader=FindElement(pElemRoot,"header");
		const ticpp::Element* const pElemType=FindElement(pElemHeader,"type");
		const ticpp::Element* const pElemApp=FindElement(pElemHeader,"application");
		const ticpp::Element* const pElemSender=FindElement(pElemHeader,"sender");
		//! wenn app leer ist und so geschrieben wurde: <application/> dann stürzt GetText() mit speicherzugriffsfehler ab...
		if(pElemApp->GetText().compare("")==0)throw ManagerException("Manager - Register: No application is empty found");
		int threadid;
		pElemID->GetText(&threadid);
		
		
		const std::string app=pElemApp->GetText();
		const std::string sender=pElemSender->GetText();
		
		if(GetClient(app,sender)!=NULL)throw ManagerException("Manager - Register: Client already exist");
		
		
		if(GetClientMap(app)==NULL)
			AddApplication(app);
		(*GetClientMap(app))[sender]=new Client(sender,threadid,(*GetSocketMap())[threadid]);
		
			
		if(sendreply)SendReply(msg);
		//std::cout<<"registriert"<<std::endl;
		
	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - Register: Xml-Error");
	}catch(ManagerException& me){
		std::string errormsg=msg;
		AddError(errormsg,me.GetError());
		if(sendreply)SendReply(errormsg);
		throw ManagerException(me.GetError());
	}	
	return 0;	
}
int Manager::Send(const std::string& msg,const bool sendreply){
	SetScope("Manager","Send");
	try{
		LOG(STR("Send: ")+=msg);
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		const ticpp::Element* pElem=doc.FirstChildElement(); //root
		const ticpp::Element* pElemHeader=FindElement(pElem,"header"); //header
		ticpp::Iterator<ticpp::Element> child;
		const std::string app=GetApp(pElem);
		//-----check the message:
		for (child=child.begin(pElemHeader);child!=child.end();child++){
			if(child.Get()->Value().compare("receiver")==0){
				if(	GetClient(app,child.Get()->GetText())==NULL){
						std::string error="No client with the name ";
						error+=child.Get()->GetText();
						Trash(msg);
						throw ManagerException(error);
				}
			}
		}
		
		//--------send the message:
		for (child=child.begin(pElemHeader);child!=child.end();child++){
			if(child.Get()->Value().compare("receiver")==0){
				GetClient(app,child.Get()->GetText())->Send(msg);
			} 	
		}
		if(sendreply)SendReply(msg);
	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - Send: Xml-Error");
	}catch(ManagerException& me){
		std::string errormsg=msg;
		AddError(errormsg,me.GetError());
		if(sendreply)SendReply(errormsg);
		throw ManagerException(me.GetError());
	}
	
	return 0;
}
int Manager::Request(const std::string& msg,const bool sendreply){
	SetScope("Manager","Request");
	//std::cout<<"Send: "<<msg<<std::endl;
	try{
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		const ticpp::Element* pElem=doc.FirstChildElement(); //root
		const ticpp::Element* pElemHeader=FindElement(pElem,"header"); //header
		ticpp::Iterator<ticpp::Element> child;
		const std::string app=GetApp(pElem);
		//-----check the message:
		for (child=child.begin(pElemHeader);child!=child.end();child++){
			if(child.Get()->Value().compare("receiver")==0){
				if(	GetClient(app,child.Get()->GetText())==NULL){
						std::string error="No client with the name ";
						error+=child.Get()->GetText();
						Trash(msg);
						throw ManagerException(error);
				}
			}
		}
		
		//--------send the message:
		for (child=child.begin(pElem);child!=child.end();child++){
			if(child.Get()->Value().compare("receiver")==0){
				//------ this is the only different from send
				GetClient(app,GetSender(pElem))->WaitingForReply(child.Get()->GetText());
				GetClient(app,GetSender(pElem))->SetReplyID(GetMsgID(pElem));
				//------
				GetClient(app,child.Get()->GetText())->Send(msg);
			} 	
		}
		if(sendreply)SendReply(msg);
	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - Send: Xml-Error");
	}catch(ManagerException& me){
		std::string errormsg=msg;
		AddError(errormsg,me.GetError());
		if(sendreply)SendReply(errormsg);
		throw ManagerException(me.GetError());
	}
	return 0;
}
int Manager::Reply(const std::string& msg,const bool sendreply){
	SetScope("Manager","Reply");
	try{
		ticpp::Element* pElem;	
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		pElem=doc.FirstChildElement(); //root
		const ticpp::Element* const pElemHeader=FindElement(pElem,"header");
		const std::string app=GetApp(pElem);
		const std::string sender=GetSender(pElem);
		const unsigned int msgid=GetReplyID(pElemHeader);
		
		if(GetClient(app,GetReceiver(pElem))->GetReplyID()==msgid)
			GetClient(app,GetReceiver(pElem))->GetReplyFrom(msg,GetSender(pElem));
		else
			throw ManagerException("Manager - Reply: Wrong ReplyID");
		
		if(sendreply)SendReply(msg);

	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - Receive: Xml-Error");
	}catch(ManagerException& me){
		std::string errormsg=msg;
		AddError(errormsg,me.GetError());
		if(sendreply)SendReply(errormsg);
		throw ManagerException(me.GetError());
	}	
	return 0;
}

int Manager::Receive(const std::string& msg,const bool sendreply){
	SetScope("Manager","Receive");
	try{
		//std::cout <<"Receive:"<<msg<<std::endl;
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		const ticpp::Element* const pElemRoot=doc.FirstChildElement(); //root
		const std::string app=GetApp(pElemRoot);
		const std::string sender=GetSender(pElemRoot);
		
		
		if(GetClient(app,sender)!=NULL){
			if(sendreply)SendReply(msg);
			GetClient(app,sender)->Receive();
		}
		else throw ManagerException("Manager - Receive: Unknown client");

	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - Receive: Xml-Error");
	}catch(ManagerException& me){
		std::string errormsg=msg;
		AddError(errormsg,me.GetError());
		if(sendreply)SendReply(errormsg);
		throw ManagerException(me.GetError());
	}	
	return 0;
}
int Manager::Count(const std::string& msg,const bool sendreply){
	SetScope("Manager","Count");
	try{
		ticpp::Element* pElem;	
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		pElem=doc.FirstChildElement(); //root
		const std::string sender=GetSender(pElem);
		const std::string app=GetApp(pElem);
		if(GetClient(app,sender)!=NULL){
			
			unsigned int size=GetClient(app,sender)->GetReceiveQueue()->size();
			if(FindElement(pElem,"text")!=NULL)	pElem->RemoveChild(FindElement(pElem,"text"));
			AddElement(pElem,"text",size);
			std::string replymsg=doc.ToString(doc);
			if(sendreply)SendReply(replymsg);
		}
		else throw ManagerException("Manager - Count: Unknown client");

	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - Count: Xml-Error");
	}catch(ManagerException& me){
		std::string errormsg=msg;
		AddError(errormsg,me.GetError());
		if(sendreply)SendReply(errormsg);
		throw ManagerException(me.GetError());
	}		
	return 0;	
}
int Manager::Disconnect(const std::string& msg,const bool sendreply){
	SetScope("Manager","Disconnect");
	//! alle die auf ne message von uns warten sollten wir befreien
	//std::cout <<"disconnect "<< msg<<std::endl;
	try{
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		const ticpp::Element* const pElemRoot=doc.FirstChildElement(); //root
		const std::string app=GetApp(pElemRoot);
		const std::string name=GetSender(pElemRoot);
		
		if(GetClient(app,name)==NULL){
			throw ManagerException("Manager - Disconnect: Unkown client");
		}else{
			delete ((*GetClientMap(app))[name]);//free the mem from the client
			(*GetClientMap(app)).erase(name);	//delete the client from the list		
			//std::cout<<"Disconnect "<<name<<" from App: "<<app<<"\n";
			if(GetClientMap(app)->empty())
				RemoveApplication(app);
			if(sendreply)SendReply(msg);			
		}
	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - XmlError: Error in Disconnect");
	}catch(ManagerException& me){
		std::string errormsg=msg;
		AddError(errormsg,me.GetError());
		if(sendreply)SendReply(errormsg);
		throw ManagerException(me.GetError());
	}	
	//std::cout <<"--"<<std::endl;
	return 0;	
}

int Manager::AddError(std::string& msg,const std::string& error){
	SetScope("Manager","AddError");
	//! totaler schwachsinn diese methode
	//std::cout <<"Error: "<<error<<"\n";
	try{
		ticpp::Element* pElem;	
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		pElem=doc.FirstChildElement(); //root
		AddElement(pElem,"error",error);
		msg=doc.ToString(doc);
	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - AddError: The message is totally brocken...\n");
	}	
	return 0;
}
int Manager::SendReply(const std::string& msg){
	SetScope("Manager","SendReply");
	try{
		
		//std::cout <<"send Reply aufgerufen"<<msg<<std::endl;
		//! wär sinnvoller hier ne neue message zu erstellen
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		const ticpp::Element* const pElemRoot=doc.FirstChildElement(); //root
		const ticpp::Element* const pElemHeader=FindElement(pElemRoot,"header");
		const ticpp::Element* const pElemType=FindElement(pElemHeader,"type");
		const std::string sender=GetSender(pElemRoot);
		const std::string app=GetApp(pElemRoot);
		
		ticpp::Element* const pElem=doc.FirstChildElement();
		//!header erst finden!!!
		if(FindElement(pElem,"type")!=NULL)	pElem->RemoveChild(FindElement(pElem,"type"));
		
		AddElement(pElem,"type","reply");
		const std::string replymsg=doc.ToString(doc);
		if(GetClient(app,sender)!=NULL){
			GetClient(app,sender)->GetSocket()->send(replymsg);
			//std::cout<<"reply :" <<replymsg<<"\n";
		}else throw ManagerException("Manager - SendReply: Unknown client");
		
	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - SendReply: XmlError");
	}catch(ManagerException& me){
		throw ManagerException(me.GetError());	
	}	
	return 0;	
}

int Manager::Log(const std::string& msg){
	SetScope("Manager","Log");
	//std::cout <<"log:\n"<<msg<<"\n\n\n";
	std::string app="";
	std::string sender="";
	bool sendmessage=true;
	try{
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		const ticpp::Element* const pElemRoot=doc.FirstChildElement(); //root
		
		sender=GetSender(pElemRoot);
		app=GetApp(pElemRoot);
		
		
		if(sender.compare(LOGNAME)==0)sendmessage=false;//if the Sender is Logger himself we dont log the message..
		
		const ticpp::Element* const pElemType=FindElement(doc.FirstChildElement(),"type");
		if(pElemType!=NULL)
			if(pElemType->GetText().compare("count")==0)sendmessage=false;
		//otherwise we log every logreceive from the logger: reiceve the log->log->receive the receive log->log->receive the receive receive log->log....
	}catch(ticpp::Exception& ex ){}//brocken message... we dont mind	
	catch(ManagerException& me){}//brocken message... we dont mind	
	//std::cout<<"log middle"<<std::endl;
	try{
		if(GetClientMap(app)!=NULL){
			if((*GetClientMap(app)).find(LOGNAME)!=(*GetClientMap(app)).end()){//if we have a "ManagerLog"-client, we send the logs to him
				while(!log.empty()){
					(*GetClientMap(app))[LOGNAME]->Send(log.front());
					log.pop_front();
				}
				if(sendmessage)(*GetClientMap(app))[LOGNAME]->Send(msg);
			}else{
				if(sendmessage)log.push_back(msg);//normal log in the logqueue
			}
		}
		if(log.size()>500)log.pop_front(); //just in case we have no logger we must delete the messages sometime
	}catch(ManagerException& me){}//we get an Exception if the messagequeue from the client is full
	//std::cout<<"log fertig\n";
	return 0;	
}
int Manager::Trash(const std::string msg){
	SetScope("Manager","Trash");
	LOG(STR("message trashed:\n")+=msg);
	trash.push_back(msg);
	if(trash.size()>500)trash.pop_front();
	return 0;	
}
unsigned int Manager::GetMsgID(const ticpp::Element* const pElem)const{
	SetScope("Manager","GetMsgID");
	int msgid=0;
	try{
		if(!ElementExist(pElem,"MsgID"))throw ManagerException("Cant Find MsgID");
		ticpp::Element* pElemMsgID=FindElement(pElem,"MsgID");
		pElemMsgID->GetText(&msgid);
		return msgid;
	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - GetMsgID: No MsgID");
	}catch(ManagerException& me){
		throw ManagerException("Manager - GetMsgID: No MsgID -> "+me.GetError());
	}
	return msgid;
}
unsigned int Manager::GetReplyID(const ticpp::Element* const pElem)const{
	SetScope("Manager","GetReplyID");
	int replyid=0;
	try{
		if(!ElementExist(pElem,"ReplyID"))throw ManagerException("Cant Find ReplyID");
		ticpp::Element* pElemMsgID=FindElement(pElem,"ReplyID");
		pElemMsgID->GetText(&replyid);
		return replyid;
	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - GetMsgID: No ReplyID");
	}catch(ManagerException& me){
		throw ManagerException("Manager - GetMsgID: No ReplyID -> "+me.GetError());
	}
	return replyid;
}
std::string Manager::GetSender(const ticpp::Element* pElem){
	SetScope("Manager","GetSender");
	std::string sender="";
	std::string app="";
	int id=0;
	if(ElementExist(pElem,"ThreadID")){
		const ticpp::Element* pElemID=FindElement(pElem,"ThreadID");
		pElemID->GetText(&id);
		sender=GetSender(id);
	}
	if(sender.compare("")==0){
		//we try to find the sender in the xml-message:
		if(ElementExist(pElem,"sender")){
			sender=FindElement(pElem,"sender")->GetText();//we cant get it with GetApp/GetSender because it could be deleted already	
		}else{
			throw ManagerException("Manager - GetSender: no sender with this name existing");
		}
	}
		
	return sender;
}
std::string Manager::GetReceiver(const ticpp::Element* pElem){
	SetScope("Manager","GetReceiver");
	std::string receiver="";
	if(!ElementExist(pElem,"receiver"))
		throw ManagerException("Manager - GetReceiver: no receiver element");
	try{
		FindElement(pElem,"receiver")->GetText(&receiver);
	}catch(ticpp::Exception& ex ){
		throw ManagerException("Manager - GetReceiver: No receiver");
	}
	return receiver;
}
std::string Manager::GetApp(const ticpp::Element* const pElem){
	SetScope("Manager","GetApp");
	std::string app="";
	int id=0;
	if(ElementExist(pElem,"ThreadID")){
		const ticpp::Element* pElemID=FindElement(pElem,"ThreadID");
		pElemID->GetText(&id);
		app=GetApp(id);
	}
	if(app.compare("")==0){
		if(ElementExist(pElem,"application")){
			app=FindElement(pElem,"application")->GetText();
		}else{
			throw ManagerException("Manager - GetApp: No application found");
		}
	}
	return app;
}
std::string Manager::GetName(const int id){
	SetScope("Manager","Name");
	std::string app="";
	std::map<std::string,std::map<std::string,Client*>* >::iterator appit;
	for(appit=clientmap.begin();appit!=clientmap.end();appit++){
		app=appit->first;
		std::map<std::string,Client*>::iterator it;
		for(it=(*GetClientMap(app)).begin();it!=(*GetClientMap(app)).end();it++){
			if( it->second->GetThreadID()==id){
				return it->second->GetName();	
			}
		}
	}
	return "";
}
std::string Manager::GetApp(const int id){
	SetScope("Manager","GetApp");
	std::string app="";
	std::map<std::string,std::map<std::string,Client*>* >::iterator appit;
	for(appit=clientmap.begin();appit!=clientmap.end();appit++){
		app=appit->first;
		std::map<std::string,Client*>::iterator it;
		for(it=(*GetClientMap(app)).begin();it!=(*GetClientMap(app)).end();it++){
			if( it->second->GetThreadID()==id){
				return app;	
			}
		}
	}
	return "";
	
}
std::string Manager::GetSender(const int id){
	SetScope("Manager","GetSender");
	return GetName(id);	
}
bool Manager::ClientRegistered(const int id){
	SetScope("Manager","ClientRegistered");
	if(GetName(id).compare("")==0)
		return false;
	else
		return true;
}
bool Manager::ClientRegistered(const ticpp::Element* const pElem){
	SetScope("Manager","ClientRegistered");
	std::string app="";
	int id=0;
	const ticpp::Element* pElemID=FindElement(pElem,"ThreadID");
	if(pElemID==NULL)
		throw ManagerException("Manager - ClientRegistered: No ThreadID exist");
	pElemID->GetText(&id);
	return ClientRegistered(id);
}
std::map<int,Socket*>* Manager::GetSocketMap(){
	SetScope("Manager","GetSocketMap");
	return &socketmap;
}
Client* Manager::GetClient(const std::string& app,const std::string& name){
	SetScope("Manager","GetClient");
	//return ((*GetClientMap())[name]);
	if(GetClientMap(app)==NULL)
		return NULL;
	if((*GetClientMap(app)).find(name)!=(*GetClientMap(app)).end())
		return ((*GetClientMap(app))[name]);
	else
		return NULL;
}
std::map<std::string,Client*>* Manager::GetClientMap(const std::string& app){
	SetScope("Manager","GetClientMap");
	if(clientmap.find(app)!=clientmap.end())
		return clientmap[app];
	else
		return NULL;
}
void Manager::AddApplication(const std::string& app){
	SetScope("Manager","AddApplication");
	if(clientmap.find(app)!=clientmap.end())
		return;
	clientmap[app]=new std::map<std::string,Client*>();
		
}
void Manager::RemoveApplication(const std::string& app){
	SetScope("Manager","RemoveApplication");
	if(clientmap.find(app)==clientmap.end())
		return;
	//! wenn die clientmap noch voll ist muss die noch gekillt werden!
	//std::cout <<"Remove application "<<app<<std::endl;
	delete clientmap[app];
	clientmap.erase(app);
}
unsigned int Manager::IDgenerator(){
	SetScope("Manager","IDgenerator");
	id_++;return id_;	
}

bool Manager::ElementExist(const ticpp::Element* pElem,const std::string& name)const {
	SetScope("Manager","ElementExist");
	if(FindElement(pElem,name)==NULL) return false;
	else return true;	
}
ticpp::Element* Manager::FindElement(const ticpp::Element* pElem,const std::string& name)const {
	SetScope("Manager","FindElement");
	ticpp::Iterator<ticpp::Element> child;
	for (child=child.begin(pElem);child!=child.end();child++){
		if(child.Get()->Value().compare(name)==0)return child.Get(); 	
	}
	return NULL;
}
int main(){
	LogMode(LOG_SHOW_NOTES);
	SetScope("","Main");
	unsigned int port=30000;
	try{
		Manager m(port);
		LOG(STR("Start Server on Port ") +="port");
		m.run();
	}catch(SocketException& e){
		
	}
return 0;
}

