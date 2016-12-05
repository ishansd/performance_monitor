# performance_monitor
Kernel module to periodically estimate performance
This uses Intel's Programmable Performance Counters (PMCs) and Fixed Function Counters.
Events that can be monitored are listed in "events.h"
Currently has 2 PMCs and 3 FFCs. 

#Usage:
make
sudo insmod perfmon.ko
sudo rmmod perfmon.ko
dmesg | tail -n
