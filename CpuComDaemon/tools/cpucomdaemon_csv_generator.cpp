/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "CpuComDaemonLog.h"
#include "Log.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

static const std::string LOGDOG_FORMAT_PATH = "vendor/melco/efw/Config/LogdogFormat";

using com::mitsubishielectric::ahu::common::FunctionID;
using namespace com::mitsubishielectric::ahu::cpucom::daemon;

int main()
{
    const std::string androidBuildTop = std::getenv("ANDROID_BUILD_TOP");
    const std::string logdogFormatFullPath = androidBuildTop + "/" + LOGDOG_FORMAT_PATH;

    const std::string cpucDaemonPath = logdogFormatFullPath + "/" + "CPUC_DAEMON.csv";
    const std::string cpucDaemonErrorPath = logdogFormatFullPath + "/" + "CPUC_DAEMON_ERROR.csv";

    InitializeCpuComLogMessages();

    using ::com::mitsubishielectric::ahu::common::DumpToCsvConfig;

    std::fstream cpucDaemonCsvFstream{cpucDaemonPath, std::ios_base::out};
    DumpToCsvConfig(FunctionID::cpuc_daemon, cpucDaemonCsvFstream);

    std::fstream cpucDaemonErrorCsvFstream{cpucDaemonErrorPath, std::ios_base::out};
    DumpToCsvConfig(FunctionID::cpuc_daemon_error, cpucDaemonErrorCsvFstream);

    TerminateCpuComLogMessages();

    return 0;
}
