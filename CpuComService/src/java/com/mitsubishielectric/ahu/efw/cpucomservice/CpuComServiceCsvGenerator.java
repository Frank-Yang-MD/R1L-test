package com.mitsubishielectric.ahu.efw.cpucomservice;

import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;

/**
 * Generates CSV files for Logdog.
 *
 * <p>Use Android Studio to run this class. In order for JNI libs to be found add
 * "-Djava.library.path=../../../../out/host/linux-x86/lib64" to VM options in Run Configuration.
 */
public class CpuComServiceCsvGenerator {
    public static void main(String[] args) {
        MLog.initialize(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.logMessageFormats);
        MLog.dumpToCsv(MLog.CPUCOM_SERVICE_FUNCTION_ID, "CPUC_SERVICE.csv");
        MLog.terminate(MLog.CPUCOM_SERVICE_FUNCTION_ID);
    }
}
