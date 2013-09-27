#ifndef XMPLIB_H
#define XMPLIB_H



#define TIXML_USE_STL
#define TIXML_USE_TICPP

#include "./tinyxml++/ticpp.h"
#include "./tinyxml++/tinyxml.h"

#ifdef USE_LIBXML
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif



namespace xmp{



class Exception{
	public:
	Exception(std::string e){	error=e;}	
	std::string GetError(){	return error;}
private:
	std::string error;
};

class XmpConnector{
public:
	XmpConnector(std::string host, int port );
	~XmpConnector();
	void Register(std::string application,std::string name);
	
	void Send(const std::string& recv,const char* const msg);
	void Send(const std::string& recv,const std::string& msg);
	void Send(const std::string& recv,const ticpp::Document& doc);
	void Request(const std::string& recv,const std::string& msg_);
	void Reply(const std::string& recv,const std::string& msgid,const std::string& msg_);
	unsigned int Count();
	void Receive();
	//config:
	void ServerReply(bool sr);
	
	//this Methods are for the last received message:
	std::string GetSender();
	std::string GetMsgID();
	std::string GetMsg();
	std::string GetType();
	std::string GetData();

#ifdef USE_LIBXML
	void GetData(xmlDocPtr& doc); 
	void GetData(xmlNodePtr& doc); 
	void GetMsg(xmlDocPtr& doc);
#endif
	ticpp::Element* GetXML(); //soll später mal GetData() heißen
	
	
private:
	bool serverreply_;
	std::string GetServerReply();
	bool ValidXML(const std::string& msg)const;
	std::string GetError(std::string& msg);
	void send(const std::string& msg)const;
	void recv(std::string& msg);
	//Socket socket;
	void* socket_prt;
	std::string lastmessage;
	std::string name;
	int randomcounter;
		
};

}
#endif
