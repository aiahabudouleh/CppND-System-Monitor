#include <string>
#include <iomanip>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) { 
  int hours = seconds / 3600 ;
  int minutes = (seconds % (3600)) / (60); 
  seconds = seconds % 60;
    return std::to_string(hours) + ":" +
           (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" +
           (seconds < 10 ? "0" : "") + std::to_string(seconds);

 }