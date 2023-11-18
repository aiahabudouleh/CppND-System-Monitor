#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
 Processor::Processor() : total_prev_(0), active_prev_(0), idle_prev_(0) {}

float Processor::Utilization() { 
    
    long total_new = LinuxParser::Jiffies();
    long active_new = LinuxParser::ActiveJiffies();
    long idle_new = LinuxParser::IdleJiffies();

    long total_old = PrevTotal();
    long idle_old = PrevIdle();

    Update(idle_new, active_new, total_new);

    float total_delta = static_cast<float>(total_new) - static_cast<float>(total_old);
    float idle_delta = static_cast<float>(idle_new) - static_cast<float>(idle_old);

    float utilization = (total_delta - idle_delta) / total_delta;
    return utilization; 
}

long Processor::PrevTotal() const { return total_prev_; }
long Processor::PrevActive() const { return active_prev_; }
long Processor::PrevIdle() const { return idle_prev_; }

void Processor::Update(long idle, long active, long total) {
        idle_prev_ = idle;
        active_prev_ = active;
        total_prev_ = total;
}