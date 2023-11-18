#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <numeric>
#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}


// TODO: Read and return the system memory utilization
/** memory utilization = (total memory - free memory) / total memory **/
float LinuxParser::MemoryUtilization() { 
  std::ifstream stream(kProcDirectory + kVersionFilename);
        std::string line, word;
        double total_memory = 0.0, free_memory = 0.0;

        if (stream.is_open()) {
            while (std::getline(stream, line)) {
                std::ifstream linestream(line);
                linestream >> word;
                if (word == "MemTotal:") {
                  linestream >> total_memory;
                } else if (word == "MemAvailable:") {
                  linestream >> free_memory;
            }              
        }
        }

        return static_cast<float>(total_memory - free_memory);
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  string line;
  long uptime;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }

  return uptime; }

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
  }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
/***
 * Read content of : proc/[pid]/stat
 * 
 * utime (index 13): Time spent in user mode
 * stime (index 14): Time spent in kernel mode
 * cutime (index 15): Time spent by waited-for children in user mode
 * cstime (index 16): Time spent by waited-for children in kernel mode
 * 
*/
long LinuxParser::ActiveJiffies(int pid) {
    long totaltime = 0;
    std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
    if (stream.is_open()) {
        std::string line;
        if (std::getline(stream, line)) {
            std::istringstream linestream(line);
            std::vector<std::string> values(std::istream_iterator<std::string>{linestream},
                                            std::istream_iterator<std::string>{});

            // Ensure correct parsing and that required values were read
            //at least 16 values were parssed 
            if (values.size() >= 17) {
                const auto is_digit = [](const std::string& str) {
                    return std::all_of(str.begin(), str.end(), isdigit);
                };

                totaltime = std::accumulate(values.begin() + 13, values.begin() + 17, 0L,
                                            [&is_digit](long acc, const std::string& val) {
                                                return is_digit(val) ? acc + stol(val) : acc;
                                            });
            }
        }
    }

    return totaltime / sysconf(_SC_CLK_TCK);
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  auto jeffies = CpuUtilization();
  const std::vector<int> cpuStates = {CPUStates::kUser_, CPUStates::kNice_,
                                    CPUStates::kSystem_, CPUStates::kIRQ_,
                                    CPUStates::kSoftIRQ_, CPUStates::kSteal_};                                  
  
  return std::accumulate(cpuStates.begin(), cpuStates.end(), 0L, [&jeffies](long acc, int state) {return acc+stol(jeffies[state]);});
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { auto jiffies = CpuUtilization();

long totalIdleJiffies = stol(jiffies[CPUStates::kIdle_]) + stol(jiffies[CPUStates::kIOwait_]);

return totalIdleJiffies; }

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line, cpu;
  vector<string> cpu_jeffies;
  if (stream.is_open()) {
    std::getline(stream, line);  
    std::istringstream linestream(line);

    linestream>>cpu;
    string jiffy;
    while(linestream>>jiffy) {
      cpu_jeffies.push_back(jiffy);
    }
  }
  return cpu_jeffies; }

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
     std::ifstream stream(kProcDirectory + kStatFilename);
     int processes = 0;
     std::string word, line;
      if (stream.is_open()) {
        while (std::getline(stream, line)) {
            std::istringstream linestream(line);
            linestream >> word;

            if (word == "processes") {
                linestream >> processes;
                break;
            }
        }
    }
  return processes; }

// TODO: Read and return the number of running processes
/**
 * file: /proc/stat
 * key: procs_running
 * 
*/
int LinuxParser::RunningProcesses() {  int runningProcesses = 0;
    std::string key, line;

    // Open the /proc/stat file for reading
    std::ifstream stream(kProcDirectory + kStatFilename);

    // Check if the file is open
    if (stream.is_open()) {
        // Read each line from the file
        while (std::getline(stream, line)) {
            // Extract the first word from the line
            std::istringstream linestream(line);
            linestream >> key;

            // Check if the key is "procs_running"
            if (key == "procs_running") {
                // If found, extract the number of running processes and break out of the loop
                linestream >> runningProcesses;
                break;
            }
        }
    }

    // Return the count of running processes
    return runningProcesses; }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
     //file:  /proc/[pid]/cmdline 
    std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
    std::string command;
    if (stream.is_open()) {
        std::getline(stream, command);
    }

    return command;
  }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function

string LinuxParser::Ram(int pid) {  
    // file:  /proc/[pid]/status file for reading
    std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
    std::string line, word, ramStr;
    long ram = 0;
    if (stream.is_open()) {

      while(std::getline(stream, line)) {
        std::istringstream linestream(line);
        linestream >> word;
        if (word == "VmSize:") {
                linestream >> ram;
                ram /= 1000;
                ramStr = std::to_string(ram);
                break;
            }        
      }
    }
    return ramStr;
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  
   // file:  /proc/[pid]/status file for reading
   std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
   std::string line, word, uid;

   if (stream.is_open()) {
        while (std::getline(stream, line)) {
            std::istringstream linestream(line);
            linestream >> word;

            if (word == "Uid:") {
                linestream >> uid;
                break;
            }
        }
    }
   return uid; }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) { 
  std::string uid = Uid(pid);
  std::ifstream stream(kPasswordPath);

  string temp, x, id, name, line;
  if (stream.is_open()) {
        while (std::getline(stream, line)) {
            std::replace(line.begin(), line.end(), ':', ' ');
            std::istringstream linestream(line);
            linestream >> temp >> x >> id;
            if (id == uid) {
                name = temp;
                break;
            }
        }
    }

  return name; }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 
    std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
    string line, value;
    std::vector<std::string> values;
     if (stream.is_open()) {
        std::getline(stream, line);
        std::istringstream linestream(line);
        while (linestream >> value) {
            values.push_back(value);
        }
    }

    long starttime;
    try {
        starttime = stol(values[21]) / sysconf(_SC_CLK_TCK);
      } catch (...) {
        // Handle any exception by setting starttime to 0
        starttime = 0;
    }
  return starttime; }
