#!/bin/bash

FILE=${ANDROID_BUILD_TOP}/out/host/linux-x86/bin/libcpucom_csv_generator

if [ -f "$FILE" ]; then
    $FILE > ${ANDROID_BUILD_TOP}/vendor/melco/efw/Config/LogdogFormat/CPUC_LIB.csv
    echo "SUCCESS. CPUC_LIB.csv created."
else 
    echo "$FILE does not exist."
    echo "Provide mm for libCpuCom"
fi
