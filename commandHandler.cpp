#include "stdlib.h"
#include "string.h"
#include "bits/stdc++.h"
#include "signal.h"
#include "iostream"
#include "unistd.h"
#include "sys/wait.h"
#include "time.h"
#include "sys/resource.h"
#include "commandHandler.h"
#include "utility.h"
using namespace std;


//exit here
bool shellExit() {
	//stop running the shell after printing
	bool runShell = false;
	int status;
	//wait until all processes are completed if any processes are active
	vector <int> allActivePid = getActiveProcesses();
	for (int i: allActivePid) {
		//resume all process to ensure any suspended process are resumed
		resumeProcess(i);
		//wait for all processes to complete
		waitForProcess(i);
	}

	cout <<"Resources used\n";
	cout <<"User time =\t"<< getUserTime() <<" seconds \n";
	cout <<"Sys  time =\t"<< getSysTime() <<" seconds\n";
	return runShell;
}

//print all jobs here
void shellJobs() {
	cout << "Running processes:\n";
	//only print processes that are actually active
	vector <int> allActivePid = getActiveProcesses();
	int count = 0;
	if (!allActivePid.empty()) {
		cout <<"#\tPID\tS\tSEC\tCOMMAND\n";
	}
	for (int activePID: allActivePid) {
		cout << count <<"\t"<<activePID<<"\t"<<getStatus(activePID)<<"\t";
		cout << getTime(activePID) << "\t";
		cout << getArgs(activePID)<<"\n";
		count ++;
	}
	cout << "Processes =\t" << count << " active\n";	
	cout << "Completed processes:\n";
	cout <<"User time =\t"<< getUserTime() <<" seconds \n";
	cout <<"Sys  time =\t"<< getSysTime() <<" seconds\n";
}

//kill specified process
void killProcess(int processPid) {
	//Check if Pid is active
	if(getActiveExistence(processPid)) {
		// kill process with given pid
		if (kill((pid_t)processPid, SIGKILL) < 0) {
			perror("Kill failed");
		}
		else {
			//remove this command from table
			removeActiveCommand(processPid);
		}
	}
	else{
		perror("No PID Found");
	}
}

//resume specified process
void resumeProcess(int processPid) {
	//Check if Pid is active
	if(getActiveExistence(processPid)) {
		//resume the process with given pid
		if (kill((pid_t)processPid, SIGCONT) < 0) {
			perror("Resume failed");
		}
		else {
			//change status of process in table
			updateActiveStatus(processPid, "R");
		}
	}
	else {
		perror("No PID Found");
	}
}

//shell sleeps for specified seconds
void shellSleep(int time) {
	//sleep for time
	sleep(time);
}

//suspend specified process
void suspendProcess(int processPid) {
	//Check if Pid is active
	if(getActiveExistence(processPid)) {
		//suspend process with given pid
		if (kill((pid_t)processPid, SIGSTOP) < 0) {
			perror("Suspend failed");
		}
		else {
			//update status of process in table
			updateActiveStatus(processPid, "S");
		}
	}
	else {
		perror("No PID Found");
	}

}

//wait for specified process to complete
void waitForProcess(int pid) {
	int status;
	//Check if Pid is active
	if(getActiveExistence(pid)) {
		//wait for process with given pid to complete
		if(waitpid((pid_t)pid, &status, 0) < 0)
			perror("Wait failed");
	}
	else {
		perror("No PID Found");
	}
}

//execute commands other than all mentioned above
void executeCommand(string rawInput) {
	string infile = "";
	string outfile = "";
	string processInput = rawInput;
	int mode = 0;
	bool background = false;

	//check for special characters like >,<, and &
	if (rawInput.find('>') < rawInput.length()) {
		size_t posOfSpecialChar = rawInput.find('>') + 1; //+1 to omit '>'
		size_t posOfSpace = rawInput.find(" ", posOfSpecialChar); 
		outfile = rawInput.substr(posOfSpecialChar, posOfSpace - posOfSpecialChar);
		//-1 to include '<'
		processInput.erase(posOfSpecialChar-1, posOfSpace-posOfSpecialChar+2);
	}
	if (rawInput.find('<') < rawInput.length()) {
		size_t posOfSpecialChar = rawInput.find('<') + 1; //+1 to omit '<'
		size_t posOfSpace = rawInput.find(" ", posOfSpecialChar); 
		infile = rawInput.substr(posOfSpecialChar, posOfSpace - posOfSpecialChar);
		//-1 to include '<'
		processInput.erase(posOfSpecialChar-1, posOfSpace-posOfSpecialChar+2); 
	}
	if (rawInput.back() == '&') {
		background = true;
		processInput.erase(processInput.end()-1, processInput.end());
	}
	
	//create a process without the special characters
	pid_t childPid = newProcess(processInput, background, infile, outfile);
	if(childPid == -1) {
		perror("Fork error");
		_exit(1);
	}
	else {
		//add to the table
		addToActiveCommand((int)childPid, rawInput);
	}
}

//spawn a new process
pid_t newProcess(string processInput, bool background, string infile, string outfile) {
	//convert string to character array
	int size = getNumberOfParams(processInput);
	char *args[size+1];
	args[size] = NULL; // should always be NULL terminated
	//convert string to char*[]
	size_t position = 0;
    int startSubString = 0;

    //spliting words in the string
    for (int i = 0; i < size; i++) {
    	position = processInput.find(" ", startSubString);
    	string interimString = processInput.substr(startSubString, position-startSubString);
    	startSubString = position + 1;
    	args[i] = strdup(interimString.c_str());
    }

	//create new process using fork
	pid_t childPid = fork();
	if(childPid == 0) {
		//if fork successful
		//do file handling
		directInFile(infile);
		directOutFile(outfile);

		if(execvp(args[0], args) < 0) {
			//exec failed
			perror("Execve Error");
			_exit(1);
		}
		_exit(1);
	}
	else {
		if(!background) {	
			//wait for child to finish if it foreground
			waitForProcess(childPid);
			return childPid;
		}
		else {
			//else do not wait until child is done
			return childPid;
		}
		
	}
}

//special code for change directory
void changeDirectory(string path) {
	char pathDir [path.length() + 1] = {};
	strcpy(pathDir, path.c_str());
	if(chdir(pathDir) < 0) {
		perror("Check directory");
	}
}

//command center of the code. Distributes actions according to the command type entered
bool shellProcess(string rawInput, int commandType) {
	bool runShell = true;
	try {
		switch (commandType) {
			case 0:
				executeCommand(rawInput);
				break;
			case 1:
				runShell = shellExit();
				break;
			case 2:
				shellJobs();
				break;
			case 3:
				killProcess(stoi(rawInput.substr(5)));
				break;
			case 4:
				resumeProcess(stoi(rawInput.substr(7)));
				break;
			case 5:
				shellSleep(stoi(rawInput.substr(6)));
				break;
			case 6:
				suspendProcess(stoi(rawInput.substr(8)));
				break;
			case 7:
				waitForProcess(stoi(rawInput.substr(5)));
				break;
			case 8:
				changeDirectory(rawInput.substr(3));
			case -1:
				//do nothing and ask for output again if someone hits enter
				break;
		}
	}
	// catch all kinds of exceptions here
	catch(...) {
		perror("Invalid arguments");
		runShell = true;
	}
	return runShell;
}
