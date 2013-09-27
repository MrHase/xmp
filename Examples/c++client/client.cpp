//using namespace xmp;
#include "client.h"
#include <iostream>

static void print_element_names(xmlNode * a_node);


int main(){
	try	{
		xmp::XmpConnector connector("localhost",30000);
		connector.Register("Example","c++client");
		connector.Send("c++client","<mydata>hi myself...></mydata>");
		connector.Receive();
		std::cout<<"We received: "<<connector.GetData()<<std::endl;
		std::cout<<"msg: "<<connector.GetMsg()<<std::endl;
		
		try{
			connector.Send("replyserver","[message]message=hello\nroom=lobby\nsender=test3\n[/message]");
		}catch(xmp::Exception  e){
			//this should fail!
			std::cout<<"lib works correctly"<<std::endl;
		}
		connector.Send("replyserver","<mydata>[message]message=hello\nroom=lobby\nsender=test3\n[/message]</mydata>");
		connector.Receive();
		std::cout<<"We received: "<<connector.GetData()<<std::endl;
		std::cout<<"msg: "<<connector.GetMsg()<<std::endl;

		std::cout<<"xml test: "<<std::endl;
		ticpp::Document doc("meine daten");
		std::string msg="<testdaten><t1>blub</t1>hallo</testdaten><t2>t2</t2>";
		doc.Parse(msg.c_str(),0,TIXML_ENCODING_UTF8);		

		connector.Send("replyserver",doc);
		connector.Receive();

		xmlNodePtr node;
		connector.GetData(node);
		print_element_names(node);
		xmlFreeNode(node);
		
	}catch(xmp::Exception e){
		std::cout<<"Error! Is the server running?"<<std::endl;
		std::cout<<"Exception: "<<e.GetError()<<std::endl;
	}
	
	return 0;	
}


static void
print_element_names(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            printf("node type: Element, name: %s\n", cur_node->name);
        }

        print_element_names(cur_node->children);
    }
}
