install:

croot && out/target/product/msm8996/vendor/app/com.mitsubishielectric.ahu.efw.cpucomservice.cpucomservicetestapp/com.mitsubishielectric.ahu.efw.cpucomservice.cpucomservicetestapp.apk
adb shell am start -n "com.mitsubishielectric.ahu.efw.cpucomservice.cpucomservicetestapp/.MainActivity"

----------------------------------------------------------------

to call CpuComService API method:

Please separate commands with COMMA!!!! for subscribeListCB and unsubscribeListCB
Whitespace don't separate commands.

adb shell am broadcast -a CpuComServiceTestApp_callApiMethod --es "method" "sendCmd" --esa "cmd" "0a,0b"
adb shell am broadcast -a CpuComServiceTestApp_callApiMethod --es "method" "subscribeCB" --esa "cmd" "f9,80"
adb shell am broadcast -a CpuComServiceTestApp_callApiMethod --es "method" "unsubscribeCB" --esa "cmd" "f9,80"
adb shell am broadcast -a CpuComServiceTestApp_callApiMethod --es "method" "subscribeListCB" --esa "cmdList" "f9,80,\ 0a,0b"
adb shell am broadcast -a CpuComServiceTestApp_callApiMethod --es "method" "unsubscribeListCB" --esa "cmdList" "f9,80,\ 0a,0b"

