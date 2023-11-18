#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid) {}

// TODO: Return this process's ID
int Process::Pid()  { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() const  { 
	 long totaltime = LinuxParser::ActiveJiffies(pid_);
  	 long pid_uptime= LinuxParser::UpTime(pid_);
  	 float pid_utilization = 0;
  	try {
  		pid_utilization = float(totaltime - pid_uptime) / pid_uptime;
  	} catch(...) {
      pid_utilization= 0 ;
    }
 	 return pid_utilization;
}

// TODO: Return the command that generated this process
string Process::Command()  { return LinuxParser::Command(pid_); }

// TODO: Return this process's memory utilization
string Process::Ram()  { return LinuxParser::Ram(pid_); }

// TODO: Return the user (name) that generated this process
string Process::User()  { return LinuxParser::User(pid_); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime()  { return LinuxParser::UpTime(pid_); }

// TODO: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
    return CpuUtilization() < a.CpuUtilization();
}
