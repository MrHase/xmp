#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <string>
#include <queue>
#include <stack>
enum LogModes{
	LOG_SHOW_NOTHING,
	LOG_SHOW_ALL,
	LOG_SHOW_NOTES,
	LOG_SHOW_WARNINGS,
	LOG_SHOW_ERRORS
};
enum LogType{
	NOTE,
	WARNING,
	ERROR
};
class SetScope_{
	public:
	SetScope_(std::string classname,std::string methodname);
	~SetScope_();
};
#define SetScope(scopeclass,scopemethod) SetScope_ justarandomvariablename(scopeclass,scopemethod);

//static void SetScope(std::string c,std::string m){}

void LOG(std::string text,LogType type=NOTE);
void LogMode(LogModes mode);
void ShowStack(); //!dazu muss der Manager als Thread laufen und immer überprüft werden ob er noch läuft

#endif
