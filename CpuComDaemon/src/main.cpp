/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include <memory>
#include <string>
#include <unordered_set>

#include <cutils/properties.h>

#include "CpuComMessageServer.h"

#include "CPU.h"
#include "CPUCommon.h"
#include "CpuComDaemon.h"
#include "DeviceConfigurations.h"
#include "EmulatorSocketDevice.h"
#include "IODevice.h"
#include "MultipleCPU.h"
#include "MutexWrapper.h"
#include "Protocol.h"
#include "UARTDevice.h"
#include "socket/SlaveDevice.h"

#include "ClientCreator.h"

#include "CpuComDaemonLog.h"
#include "LibVehiclePwrLog.h"
#include "Log.h"
#include "libMelcoCommon.h"

namespace common = com::mitsubishielectric::ahu::common;
namespace cpucom = com::mitsubishielectric::ahu::cpucom;
namespace impl = com::mitsubishielectric::ahu::cpucom::impl;
namespace vehiclepwrmgrLib = com::mitsubishielectric::ahu::vehiclepwrmgr::lib;
namespace socket = com::mitsubishielectric::ahu::cpucom::impl::socket;

using com::mitsubishielectric::ahu::common::IExecutor;
using com::mitsubishielectric::ahu::common::IODevice;
using com::mitsubishielectric::ahu::common::MLOGD;
using com::mitsubishielectric::ahu::common::MLOGW;
using com::mitsubishielectric::ahu::common::PeriodicTaskExecutor;
using com::mitsubishielectric::ahu::common::SingleThreadExecutor;
using com::mitsubishielectric::ahu::vehiclepwrmgr::lib::ClientCreator;
using com::mitsubishielectric::ahu::vehiclepwrmgr::lib::ICpuComClient;

#define USE_2_UART

namespace {
#ifdef USE_2_UART
constexpr bool kUse2Uart = true;
#else
constexpr bool kUse2Uart = false;
#endif
}  // namespace

void onCpuComDaemonStarted()
{
    MLOGD(common::FunctionID::cpuc_daemon, cpucom::daemon::LogID::Started);
    auto vpm = std::unique_ptr<ICpuComClient>(ClientCreator::CreateCpuComClient());

    if (vpm) {
        vpm->CpuComStarted();
    }
    else {
        MLOGW(common::FunctionID::cpuc_daemon, cpucom::daemon::LogID::VPMerror);
    }
}

std::unique_ptr<impl::CPU> getVcpuEmu(impl::EmulatorSocketDevice& emulatorDevice)
{
    cpucom::DeviceConfigureEmulatorSocket configureEmulatorSocket(emulatorDevice);

    auto device{std::make_unique<socket::SlaveDevice>(impl::kVCPUEmulatorSocketName,
                                                      configureEmulatorSocket)};
    auto protocol = std::make_unique<impl::Protocol>(std::move(device));
    return std::make_unique<impl::CPU>(std::move(protocol), impl::kAddressVCPU);
}

std::unique_ptr<impl::ICPU> getRealCpu(impl::UARTDevice& uartDevice)
{
    cpucom::DeviceConfigureUART configureUART(uartDevice);
    std::unique_ptr<impl::ICPU> vcpu;
    if (kUse2Uart) {
        auto deviceReceive{std::make_unique<common::IODevice>("/dev/ttyHS4", configureUART)};
        auto deviceTransmit{std::make_unique<common::IODevice>("/dev/ttyHS6", configureUART)};

        auto protocolReceive = std::make_unique<impl::Protocol>(std::move(deviceReceive));
        auto protocolTransmit = std::make_unique<impl::Protocol>(std::move(deviceTransmit));

        vcpu = std::make_unique<impl::MultipleCPU>(std::move(protocolReceive),
                                                   std::move(protocolTransmit), impl::kAddressVCPU);
    }
    else {
        auto device{std::make_unique<common::IODevice>(impl::kUartDeviceName, configureUART)};
        auto protocol = std::make_unique<impl::Protocol>(std::move(device));
        vcpu = std::make_unique<impl::CPU>(std::move(protocol), impl::kAddressVCPU);
    }

    return vcpu;
}

void onCpuComDaemonStopped()
{
    MLOGD(common::FunctionID::cpuc_daemon, cpucom::daemon::LogID::Stopped);
}

int main()
{
    common::InitializeCommonLogMessages();
    cpucom::daemon::InitializeCpuComLogMessages();
    vehiclepwrmgrLib::InitializeLibVehiclePwrLogMessages();

    bool useSocketDevice = static_cast<bool>(property_get_bool("vendor.vcpuemulator", 0));

    std::unique_ptr<impl::ICPU> vcpu;

    impl::UARTDevice uartDevice;
    impl::EmulatorSocketDevice emulatorDevice;

    if (useSocketDevice) {
        vcpu = getVcpuEmu(emulatorDevice);
    }
    else {
        vcpu = getRealCpu(uartDevice);
    }

    std::unique_ptr<IExecutor> incomingExecutor = std::make_unique<SingleThreadExecutor>();
    std::unique_ptr<IExecutor> outcomingExecutor = std::make_unique<SingleThreadExecutor>();
    std::unique_ptr<impl::IMessageServer> messageServer =
        std::make_unique<impl::CpuComMessageServer>(
            impl::kCpuComDaemonSocketName,
            std::unordered_set<std::string>{"u:r:cpucomservice_app:s0"},
            std::move(incomingExecutor), std::move(outcomingExecutor), onCpuComDaemonStarted,
            onCpuComDaemonStopped);

    SingleThreadExecutor singleExecutor;
    auto periodicExecutor = std::make_unique<PeriodicTaskExecutor>(singleExecutor);

    auto subscribersMutexWrapper = std::make_unique<cpucom::MutexWrapper>();
    auto requestsMutexWrapper = std::make_unique<cpucom::MutexWrapper>();

    cpucom::CpuComDaemon daemon(std::move(messageServer), std::move(vcpu),
                                std::move(periodicExecutor), std::move(subscribersMutexWrapper),
                                std::move(requestsMutexWrapper));
    bool result = daemon.start();

    vehiclepwrmgrLib::TerminateLibVehiclePwrLogMessages();
    cpucom::daemon::TerminateCpuComLogMessages();
    common::TerminateCommonLogMessages();
    return static_cast<int>(!result);
}
