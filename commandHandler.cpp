#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include "iostream"
#include "unistd.h"
#include "sys/wait.h"
#include "commandHandler.h"
#include "utility.h"
using namespace std;

bool shellExit() {
	//exit here
	return false;
}

void shellJobs() {
	//print all jobs here
}

void killProcess(int processPid) {
	//Check if Pid is active
	if(getExistence(processPid))
		// kill process with given pid
		kill((pid_t)processPid, SIGKILL);
	else
		cout<<getError();
}

void resumeProcess(int processPid) {
	//Check if Pid is active
	if(getExistence(processPid))	
		//resume the process with given pid
		kill((pid_t)processPid, SIGCONT);
	else
		cout<<getError();
}

void shellSleep(int time) {
	//sleep for time
	sleep(time);
}

void suspendProcess(int processPid) {
	//Check if Pid is active
	if(getExistence(processPid))
		//suspend process with given pid
		kill((pid_t)processPid, SIGSTOP);
	else
		cout<<getError();

}

void waitForProcess(int processPid) {
	//Check if Pid is active
	if(getExistence(processPid)) {
		//wait for process with given pid to complete

	}
	else
		cout<<getError();
}

void executeCommand(string raw_input) {
	//process these special commands
}

bool shellProcess(string raw_input, int commandType) {
	bool runShell = true;
	try {
		switch (commandType) {
			case 0:
				executeCommand(raw_input);
				break;
			case 1:
				runShell = shellExit();
				break;
			case 2:
				shellJobs();
				break;
			case 3:
				killProcess(stoi(raw_input.substr(5)));
				break;
			case 4:
				resumeProcess(stoi(raw_input.substr(7)));
				break;
			case 5:
				shellSleep(stoi(raw_input.substr(6)));
				break;
			case 6:
				suspendProcess(stoi(raw_input.substr(8)));
				break;
			case 7:
				waitForProcess(stoi(raw_input.substr(5)));
			case -1:
				//do nothing and ask for output again if someone hits enter
				break;
		}
	}
	// catch all kinds of exceptions
	catch(...) {
		cout << getError();
		runShell = true;
	}
	return runShell;
}