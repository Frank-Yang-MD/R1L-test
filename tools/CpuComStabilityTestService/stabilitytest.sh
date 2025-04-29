#!/system/bin/sh
logcat -c
am startservice com.mitsubishielectric.ahu.efw.cpucomstabilitytestservice/.CpuComStabilityTestService
pid=$(echo $(ps -eoPID,NAME | grep -iE "(cpucomstabilitytestservice)$") | cut -d" " -f1)
logcat --pid $pid
exit 0