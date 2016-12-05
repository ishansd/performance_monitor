# performance_monitor
Kernel module to periodically estimate performance
This uses Intel's Programmable Performance Counters (PMCs) and Fixed Function Counters.
Events that can be monitored are listed in "events.h"
Currently has 2 PMCs and 3 FFCs. 

Usage:
Make module with
#make
Load kernel module (perfmon.ko) with:
#sudo insmod perfmon.ko
Remove module with:
#sudo rmmod perfmon.ko
Check log with (for n lines):
#dmesg | tail -n
