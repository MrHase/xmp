#include "Log.h"

#include <mutex>
#include <thread>
#include <map>
#include <sstream>

using namespace std;

class Scope{
public:
	std::string classname;
	std::string methodname;
	
	Scope(std::string cn, std::string mn){
		classname=cn;
		methodname=mn;
	}
};

std::mutex logging_mutex;
static std::stack<Scope> scopequeue;
static map<string,stack<Scope>> scopestacks;

static bool LogType_NOTE=false,LogType_WARNING=false,LogType_ERROR=false;

SetScope_::SetScope_(std::string classname, std::string methodname){
    logging_mutex.lock();

    std::thread::id thread_id_ = std::this_thread::get_id();
    stringstream thread_id;
    thread_id<<thread_id_;
    //std::cout<< "THREAD ID: "<<thread_id<<std::endl;

	Scope scope(classname,methodname);
	scopequeue.push(scope);
    scopestacks[thread_id.str()].push(scope);
	//std::cout<<scopequeue.size()<<std::endl;
    logging_mutex.unlock();
}
SetScope_::~SetScope_(){
    logging_mutex.lock();
	//std::cout<<"destructor"<<std::endl;
    std::thread::id thread_id_ = std::this_thread::get_id();
    stringstream thread_id;
    thread_id<<thread_id_;

	scopequeue.pop();
    scopestacks[thread_id.str()].pop();
    logging_mutex.unlock();
}


void LOG(std::string text,LogType type){

    logging_mutex.lock();
    std::thread::id thread_id_ = std::this_thread::get_id();
    stringstream thread_id;
    thread_id<<thread_id_;

    if(type==WARNING && LogType_WARNING ||type==NOTE && LogType_NOTE || type==ERROR && LogType_ERROR)
        std::cout <<scopestacks[thread_id.str()].top().classname<<"::"<<scopestacks[thread_id.str()].top().methodname<<" -> "<<text<<std::endl;

    logging_mutex.unlock();

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
