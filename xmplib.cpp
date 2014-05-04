#include "xmplib.hpp"
#include "Socket.h"
#include "SocketException.h"
#include "math.h"


using namespace xmp;
//-----------------Usefull functions:
ticpp::Element* FindElement(const ticpp::Element* pElem,const std::string& name){
	if (!pElem)
		return NULL;
	ticpp::Iterator<ticpp::Element> child;
	for (child=child.begin(pElem);child!=child.end();child++){
		if(child.Get()->Value().compare(name)==0)return child.Get(); 	
	}
	return NULL;
}
ticpp::Element* FirstElement(const ticpp::Element* pElem){
	if (!pElem)
		return NULL;
	ticpp::Iterator<ticpp::Element> child;
	for (child=child.begin(pElem);child!=child.end();child++){
		return child.Get(); 	
	}
	return NULL;
}
unsigned int HexToChar(std::string str){
	//std::cout <<"HexToChar: str: "<<str<<std::endl;
	unsigned int ret=0;
	for(int i=0;i<str.length();i++){
		unsigned int mult=pow(16,str.length()-i-1);
		switch( str.at(i)){
			case '0':ret+=0*mult;break;
			case '1':ret+=1*mult;break;
			case '2':ret+=2*mult;break;
			case '3':ret+=3*mult;break;
			case '4':ret+=4*mult;break;
			case '5':ret+=5*mult;break;
			case '6':ret+=6*mult;break;
			case '7':ret+=7*mult;break;
			case '8':ret+=8*mult;break;
			case '9':ret+=9*mult;break;
			case 'A':ret+=10*mult;break;
			case 'B':ret+=11*mult;break;
			case 'C':ret+=12*mult;break;
			case 'D':ret+=13*mult;break;			
			case 'E':ret+=14*mult;break;
			case 'F':ret+=15*mult;break;
			default:
			std::cout << "ERROR"<<std::endl;
			//! exception
		}
	}
	return ret;
}
std::string ElementToString(const ticpp::Element* pElem){
	size_t pos=0;
	TiXmlPrinter printer;
	//printer.SetIndent( "\t" );
	pElem->Accept( &printer );
	std::string str=printer.Str();

	
	while(str.find('&',pos)!=std::string::npos){
		pos=str.find('&',pos);
	
		if (str.length()-pos>=4){
			if(str.substr(pos,4).compare("&lt;")==0)
				str.replace(pos,4,"<");
			if(str.substr(pos,4).compare("&gt;")==0)
				str.replace(pos,4,">");
		}
		if (str.length()-pos>=5){
			if(str.substr(pos,5).compare("&amp;")==0)
				str.replace(pos,5,"&");
			
		}
		if (str.length()-pos>=6){
			if(str.substr(pos,6).compare("&quot;")==0)
				str.replace(pos,6,"\"");
		
			if(str.substr(pos,6).compare("&apos;")==0)
				str.replace(pos,6,"'");
			
			if(str.substr(pos,2).compare("&#")==0){
				char c='\0';
				if(str.substr(pos+2,1).compare("x")==0){
					//hex
					c=(char)HexToChar(str.substr(pos+3,2));
				}else{
					//int
					c=(char) atoi(str.substr(pos+2,3).c_str());
				}
				//printf("replace with: %d\n",c);
				char c_str[2];// geht nur mit c++0x or gnu++0x :{c,'\0'};
				c_str[0]=c;
				c_str[1]='\0';
				str.replace(pos,6,std::string(c_str));
			}	
		}
		pos++;
	}
	return str;
}

static xmlNodePtr GetFirst(xmlNodePtr a_node,std::string name){
	
	xmlNode *cur_node = NULL;
    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
			std::cout <<"searching for: "<<name<<" found: "<<cur_node->name<<"\n";
			if(strcmp((const char*)cur_node->name,name.c_str())==0){
				return cur_node;
			}
        }
    }  
	
}


//-------------------------------------------------------------------
XmpConnector::XmpConnector( std::string host, int port )
{
	socket_prt= (void*) new Socket();
	
	Socket& socket= (* (Socket*)socket_prt);
	lastmessage="";
	randomcounter=0;
    serverreply_=false;
	if ( ! socket.create() )
	{
		throw Exception ( "Could not create client socket." );
	}
	if ( ! socket.connect ( host, port ) )
	{
		throw Exception ( "Could not bind to port." );
	}

}
XmpConnector::~XmpConnector(){
	
	delete (Socket*)socket_prt;
}
void XmpConnector::Register(std::string application,std::string regname){

		
	Socket& socket= (* (Socket*)socket_prt);
	
	name=regname;
	
	std::string sendmsg;
	//<type reply=yes>register</type>\
	
	sendmsg="<msg><header>\
		<type reply=\""+GetServerReply()+"\">register</type><sender>"+name+"</sender><application>"+application+"</application>\
		</type></header></msg>";
	//macht hier wenig sinn, da wenn wir uns nicht registrieren können, bekommen wir kein reply...
	try{
		socket.send(sendmsg);
		if(serverreply_){
			std::string serverreply;
			socket.recv(serverreply);
			if(!GetError(serverreply).empty())
				throw Exception(GetError(serverreply));
		}				
	}catch(SocketException& se){
		throw Exception("Error: Network");
	}catch(ticpp::Exception& se){
		throw Exception("Error: Xml");
	}
	
}

void  XmpConnector::Send(const std::string& recv,const char* const msg){
	Send(recv,std::string(msg));	
}
void XmpConnector::Send(const std::string& recv,const std::string& msg_){
	if (!ValidXML(msg_))
		throw Exception("No valid xml");
	
	Socket& socket= (* (Socket*)socket_prt);
	//std::cout <<"msg to send: "<<msg_<<std::endl;	
	std::string sendmsg;
	sendmsg="<msg><header><type reply=\""+GetServerReply()+"\">normal</type>\
	<receiver>"+recv+"</receiver>\
	</header><data>"+msg_+"</data></msg>";
	try{
		
		socket.send(sendmsg);
		if(serverreply_){
			std::string serverreply;
			socket.recv(serverreply);
			if(!GetError(serverreply).empty())
				throw Exception(GetError(serverreply));
		}			
		
				
	}catch(SocketException& se){
		throw Exception("Error: Network");
	}catch(ticpp::Exception& se){
		throw Exception("Error: Xml");
	}
}

void XmpConnector::Send(const std::string& recv,const ticpp::Document& doc){
	Socket& socket= (* (Socket*)socket_prt);
	
	std::string sendmsg;
	sendmsg="<msg><header><type reply=\""+GetServerReply()+"\">normal</type>\
	<receiver>"+recv+"</receiver>\
	</header><data>"+doc.ToString(doc)+"</data></msg>";
	try{
		socket.send(sendmsg);
		if(serverreply_){
			std::string serverreply;
			socket.recv(serverreply);
			if(!GetError(serverreply).empty())
				throw Exception(GetError(serverreply));
		}			
	}catch(SocketException& se){
		throw Exception("Error: Network");
	}catch(ticpp::Exception& se){
		throw Exception("Error: Xml");
	}		

}
void XmpConnector::Request(const std::string& recv,const std::string& msg_){

	if (!ValidXML(msg_))
		throw Exception("No valid xml");
		
	Socket& socket= (* (Socket*)socket_prt);
	
	std::string sendmsg;
	sendmsg="<msg><header><type reply=\""+GetServerReply()+"\">request</type>\
	<receiver>"+recv+"</receiver>\
	</header><data>"+msg_+"</data></msg>";
	try{
		socket.send(sendmsg);
		if(serverreply_){
			std::string serverreply;
			socket.recv(serverreply);//serverreply
			if(!GetError(serverreply).empty())
				throw Exception(GetError(serverreply));
		}
		socket.recv(lastmessage);//reply from another client
						
	}catch(SocketException& se){
		throw Exception("Error: Network");
	}catch(ticpp::Exception& se){
		throw Exception("Error: Xml");
	}	
}
void XmpConnector::Reply(const std::string& recv,const std::string& msgid,const std::string& msg_){
	
	if (!ValidXML(msg_))
		throw Exception("No valid xml");
	Socket& socket= (* (Socket*)socket_prt);
	std::string sendmsg;
	//achtung der folgende kommentar geht bis zum semikolon
	sendmsg="<msg><header><type reply=\""+GetServerReply()+"\">reply</type>\
	<receiver>"+recv+"</receiver>\
	<ReplyID>"+msgid+"</ReplyID></header><data>"\
	+msg_+"</data></msg>";
	
	try{
		socket.send(sendmsg);
		if(serverreply_){
			std::string serverreply;
			socket.recv(serverreply);
			if(!GetError(serverreply).empty())
				throw Exception(GetError(serverreply));
		}
						
	}catch(SocketException& se){
		throw Exception("Error: Network");
	}catch(ticpp::Exception& se){
		throw Exception("Error: Xml");
	}	
	
}
unsigned int XmpConnector::Count(){
	Socket& socket= (* (Socket*)socket_prt);
	unsigned int ret=0;
	std::string sendmsg;
	//even if we set ServerReply to false its stupid here...
	sendmsg="<msg><header><type reply=\"yes\">count</type></header></msg>";
	try{
		
		socket.send(sendmsg);
		std::string serverreply;
		socket.recv(serverreply);
		if(!GetError(serverreply).empty())
			throw Exception(GetError(serverreply));
		
		try{
			ticpp::Element* pElem;	
			ticpp::Document doc("msg");
			doc.Parse(serverreply.c_str(),0,TIXML_ENCODING_UTF8);
			pElem=doc.FirstChildElement(); //root
			pElem=FindElement(pElem,"text");
			
			if(pElem!=NULL){
				pElem->GetText(&ret);
			}
		}catch(ticpp::Exception& e){
			throw Exception("Error: Xml");	
		}
				
	}catch(SocketException& se){
		throw Exception("Error: Network");
	}catch(ticpp::Exception& se){
		throw Exception("Error: Xml");
	}	
	return ret;
}
void XmpConnector::Receive(){
	Socket& socket= (* (Socket*)socket_prt);
	
	std::string sendmsg;
	sendmsg="<msg><header><type reply=\""+GetServerReply()+"\">receive</type>\
	</header></msg>";
	try{
		socket.send(sendmsg);
		if(serverreply_){
			std::string serverreply;
			socket.recv(serverreply);//serverreply
			if(!GetError(serverreply).empty())
				throw Exception(GetError(serverreply));
		}
		socket.recv(lastmessage);
		
	}catch(SocketException& se){
		throw Exception("Error: Network");
	}catch(ticpp::Exception& se){
		throw Exception("Error: Xml");
	}	
}
void XmpConnector::ServerReply(bool sr){
	serverreply_=sr;	
}
//this Methods are for the last received message:
std::string XmpConnector::GetSender(){
	std::string ret;
	try{
		ticpp::Element* pElem;	
		ticpp::Document doc("msg");
		doc.Parse(lastmessage.c_str(),0,TIXML_ENCODING_UTF8);
		pElem=doc.FirstChildElement(); //root
		pElem=FindElement(pElem,"sender");
		if(pElem!=NULL){
			pElem->GetText(&ret);
		}
	}catch(ticpp::Exception& e){
		throw Exception("Error: Xml");	
	}
	return ret;
}
std::string XmpConnector::GetMsgID(){
	std::string ret;
	try{
		ticpp::Element* pElem;	
		ticpp::Document doc("msg");
		doc.Parse(lastmessage.c_str(),0,TIXML_ENCODING_UTF8);
		pElem=doc.FirstChildElement(); //root
		pElem=FindElement(pElem,"MsgID");
		if(pElem!=NULL){
			pElem->GetText(&ret);
		}
	}catch(ticpp::Exception& e){
		throw Exception("Error: Xml");	
	}
	return ret;
}
std::string XmpConnector::GetMsg(){
	return lastmessage;
}
std::string XmpConnector::GetType(){
	std::string ret;
	try{
		ticpp::Element* pElem;	
		ticpp::Document doc("msg");
		doc.Parse(lastmessage.c_str(),0,TIXML_ENCODING_UTF8);
		pElem=doc.FirstChildElement(); //root
		pElem=FindElement(pElem,"header");
		pElem=FindElement(pElem,"type");
		if(pElem!=NULL){
			pElem->GetText(&ret);
		}
	}catch(ticpp::Exception& e){
		throw Exception("Error: Xml");	
	}
	return ret;
}

void XmpConnector::GetData(xmlDocPtr& doc){
	  
	//! muss noch gemacht werden...
    doc = xmlReadMemory(lastmessage.c_str(),lastmessage.length() , "noname.xml", NULL, 0); //! wie verhält sich die length bei utf8?
    if (doc == NULL) {
		//! exception
        fprintf(stderr, "Failed to parse document\n");
    }
}

void XmpConnector::GetData(xmlNodePtr& node){
	xmlDocPtr doc;
	doc = xmlReadMemory(lastmessage.c_str(),lastmessage.length() , "noname.xml", NULL, 0); //! wie verhält sich die length bei utf8?
    if (doc == NULL) {
		//! exception
        throw Exception ("Failed to parse document");
    }
    
    xmlNodePtr a_node=xmlDocGetRootElement(doc);
	xmlNodePtr tmp;
	tmp=GetFirst(a_node,"data");
    node=xmlCopyNodeList(tmp->children);//! muss noch gemacht werden
	
	xmlFreeDoc(doc);
    if(node==NULL)
		throw Exception("Error: libxml2 can't copy the nodelist");	
}
void XmpConnector::GetMsg(xmlDocPtr& doc){
	  
    doc = xmlReadMemory(lastmessage.c_str(),lastmessage.length() , "noname.xml", NULL, 0); //! wie verhält sich die length bei utf8?
    if (doc == NULL) {
		//! exception
        fprintf(stderr, "Failed to parse document\n");
    }
}

std::string XmpConnector::GetData(){
	std::string ret;

	try{
	
		ticpp::Element* pElem;	
		ticpp::Document doc("msg");
		doc.Parse(lastmessage.c_str(),0,TIXML_ENCODING_UTF8);
		pElem=doc.FirstChildElement(); //root
		
		pElem=FirstElement(FindElement(pElem,"data"));
		if(pElem!=NULL){
			ret=ElementToString(pElem);
		}else{
			return "";//we have no data...
		}
				
	}catch(ticpp::Exception& e){
		throw Exception("Error: Xml");	
	}
	return ret;
}

ticpp::Element* XmpConnector::GetXML(){
	ticpp::Document data();
	try{
		ticpp::Element* pElem;	
		ticpp::Document doc("msg");
		doc.Parse(lastmessage.c_str(),0,TIXML_ENCODING_UTF8);
		pElem=doc.FirstChildElement(); //root
		pElem=FindElement(pElem,"data");
		if(pElem!=NULL){
			std::cout <<"---- "<<pElem<<std::endl;
			return ((pElem->Clone()->ToElement())); //! existiert das nach der rückgabe überhaupt noch?
		}
	}catch(ticpp::Exception& e){
		throw Exception("Error: Xml");	
	}
}
std::string XmpConnector::GetError(std::string& msg){
	//! fehlt hier nicht noch der try/catch block?
	std::string error="";
	//std::cout<<"GetError: "<<msg<<"\n";
	ticpp::Element* pElem;	
	ticpp::Document doc("msg");
	doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
	pElem=doc.FirstChildElement(); //root
	pElem=FindElement(pElem,"error");
	
	if(pElem!=NULL){
		//pElem->GetText(&error);
		error=msg;
	}

	return error;
}
std::string XmpConnector::GetServerReply(){
	if(serverreply_)	return "yes";
	return "no";
	
}

bool XmpConnector::ValidXML(const std::string& msg)const{
	//! todo
	try{
		ticpp::Element* pElem;	
		ticpp::Document doc("msg");
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);
		pElem=doc.FirstChildElement(); //root
	}catch(ticpp::Exception& e){
		return false;	
	}
	return true;
}

void XmpConnector::send(const std::string& msg)const {
	Socket& socket= (* (Socket*)socket_prt);
	//! hier sollte die msg noch mal überprüft werden ob alle
	//! wichtigen daten enthalten sind
  if ( ! socket.send ( msg ) )
    {
      throw SocketException ( "Could not write to socket." );
    }
}

void XmpConnector::recv(std::string& msg){
	Socket& socket= (* (Socket*)socket_prt);
	//! hier sollte die msg noch mal überprüft werden ob alle
	//! wichtigen daten enthalten sind
  if ( ! socket.recv ( msg ) )
    {
      throw SocketException ( "Could not read from socket." );
    }
}
