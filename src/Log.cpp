#include "Log.h"

class Scope{
public:
	std::string classname;
	std::string methodname;
	
	Scope(std::string cn, std::string mn){
		classname=cn;
		methodname=mn;
	}
};
static std::stack<Scope> scopequeue;

static bool LogType_NOTE=false,LogType_WARNING=false,LogType_ERROR=false;

SetScope_::SetScope_(std::string classname, std::string methodname){
	Scope scope(classname,methodname);
	scopequeue.push(scope);
	//std::cout<<scopequeue.size()<<std::endl;
}
SetScope_::~SetScope_(){
	//std::cout<<"destructor"<<std::endl;
	scopequeue.pop();
}


void LOG(std::string text,LogType type){
	//std::cout << scopequeue.size()<<std::endl;
	//if(scopequeue.front()!=NULL)
	if(type==WARNING && LogType_WARNING ||type==NOTE && LogType_NOTE || type==ERROR && LogType_ERROR)
		std::cout <<scopequeue.top().classname<<"::"<<scopequeue.top().methodname<<" -> "<<text<<std::endl;	
}
void LogMode(LogModes mode){
	switch(mode){
		case LOG_SHOW_ALL:
			LogType_NOTE=true;
			LogType_WARNING=true;
			LogType_ERROR=true;
			break;
		case LOG_SHOW_NOTHING:
			LogType_NOTE=false;
			LogType_WARNING=false;
			LogType_ERROR=false;
			break;
		case LOG_SHOW_NOTES:
			LogType_NOTE=true;
			break;
		case LOG_SHOW_WARNINGS:
			LogType_WARNING=true;
			break;
		case LOG_SHOW_ERRORS:
			LogType_ERROR=true;
			break;
		default:
			break;
	}
}
/*
void test2();
void test1(){
	SetScope("test1");
	Log("before t2");
	test2();
	Log("after t2");
}
void test2(){
	SetScope ("test2");
	Log("test2");
}
int main(){
	std::cout<<"los"<<std::endl;;
	
	SetScope("Main");
	Log("start");
	test1();
	Log("exit");
	
	return 0;
}
*/
