/*
 * COPYRIGHT (C) 2021 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "Log.h"
#include "libCpuCom.h"

#include <iostream>

int main()
{
    using com::mitsubishielectric::ahu::cpucom::InitializeLibCpuComLogMessages;
    InitializeLibCpuComLogMessages();

    using com::mitsubishielectric::ahu::common::DumpToCsvConfig;
    using com::mitsubishielectric::ahu::common::FunctionID::cpuc_lib;
    DumpToCsvConfig(cpuc_lib, std::cout);

    using com::mitsubishielectric::ahu::cpucom::TerminateLibCpuComLogMessages;
    TerminateLibCpuComLogMessages();

    return 0;
}
