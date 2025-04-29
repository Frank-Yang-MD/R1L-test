/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include <algorithm>
#include <chrono>
#include <deque>
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>

#include "CPU.h"
#include "CpuCommand.h"
#include "DeviceConfigurations.h"
#include "EmulatorSocketDevice.h"
#include "Log.h"
#include "MasterDevice.h"
#include "Protocol.h"
#include "RulesBuilder.h"
#include "ThreadPool.h"
#include "UARTDevice.h"

#include "CpuComDaemonLog.h"
#include "libMelcoCommon.h"

#include "CPUCommon.h"

#include "Emulator.h"
#include "Utils.h"

#include "EmulatorMessage.h"
#include "Executors.h"
#include "messenger/MessageServer.h"

using com::mitsubishielectric::ahu::common::CpuCommand;
using com::mitsubishielectric::ahu::common::InitializeCommonLogMessages;
using com::mitsubishielectric::ahu::common::IODevice;
using com::mitsubishielectric::ahu::common::MessageServer;
using com::mitsubishielectric::ahu::common::MLOGD_SERIAL;
using com::mitsubishielectric::ahu::common::SingleThreadExecutor;
using com::mitsubishielectric::ahu::common::TerminateCommonLogMessages;
using com::mitsubishielectric::ahu::common::ThreadPool;
using com::mitsubishielectric::ahu::cpucom::DeviceConfigureEmulatorSocket;
using com::mitsubishielectric::ahu::cpucom::DeviceConfigureUART;
using com::mitsubishielectric::ahu::cpucom::daemon::InitializeCpuComLogMessages;
using com::mitsubishielectric::ahu::cpucom::daemon::TerminateCpuComLogMessages;
using com::mitsubishielectric::ahu::cpucom::impl::CPU;
using com::mitsubishielectric::ahu::cpucom::impl::Emulator;
using com::mitsubishielectric::ahu::cpucom::impl::IRulesBuilder;
using com::mitsubishielectric::ahu::cpucom::impl::kUartDeviceName;
using com::mitsubishielectric::ahu::cpucom::impl::kVCPUEmulatorSocketName;
using com::mitsubishielectric::ahu::cpucom::impl::Protocol;
using com::mitsubishielectric::ahu::cpucom::impl::RulesBuilder;
using com::mitsubishielectric::ahu::cpucom::impl::VcpuEmulator;
using com::mitsubishielectric::ahu::cpucom::impl::socket::MasterDevice;

using com::mitsubishielectric::ahu::cpucom::impl::EmulatorId;
using com::mitsubishielectric::ahu::cpucom::impl::kEmulatorCLISocketName;

using namespace com::mitsubishielectric::ahu::cpucom;

using namespace com::mitsubishielectric::ahu::cpucom::utils;

int main()
{
    InitializeCommonLogMessages();
    InitializeCpuComLogMessages();

    impl::EmulatorSocketDevice emulatorDevice;
    DeviceConfigureEmulatorSocket configureEmulatorSocket(emulatorDevice);
    auto device = std::make_unique<MasterDevice>(kVCPUEmulatorSocketName, configureEmulatorSocket);
    auto protocol = std::make_unique<Protocol>(std::move(device));
    auto mcpu = std::make_shared<CPU>(std::move(protocol), impl::kAddressMCPU);

    impl::UARTDevice uartDevice;
    DeviceConfigureUART configureUART(uartDevice);
    auto vcpudevice = std::make_unique<IODevice>(kUartDeviceName, configureUART);
    auto vcpuprotocol = std::make_unique<Protocol>(std::move(vcpudevice));
    auto vcpu = std::make_shared<CPU>(std::move(vcpuprotocol), impl::kAddressVCPU);

    auto rulesBuilder = std::make_unique<RulesBuilder>(mcpu, vcpu);
    auto emulator = std::make_unique<Emulator>(std::move(rulesBuilder), mcpu, vcpu);

    auto incomingExecutor = std::make_unique<SingleThreadExecutor>();
    auto outcomingExecutor = std::make_unique<SingleThreadExecutor>();

    MessageServer<EmulatorId> messenger{kEmulatorCLISocketName, std::move(incomingExecutor),
                                        std::move(outcomingExecutor)};

    emulator->start();
    MLOGD_SERIAL("[emulator]", "Started\n");

    std::function<void(MessageServer<EmulatorId>::SessionID, std::string)> handler =
        [&emulator](MessageServer<EmulatorId>::SessionID, std::string input) {
            std::deque<std::string> tokens = tokenize(input, ' ');
            if (!tokens.empty()) {
                std::string what = tokens.front();
                tokens.pop_front();
                if (what == "send") {
                    if (!tokens.empty()) {
                        std::string commandString = tokens.front();
                        tokens.pop_front();
                        std::pair<uint8_t, uint8_t> command;
                        bool commandParsed = commandFromString(commandString, command);
                        if (commandParsed) {
                            std::vector<uint8_t> data;
                            if (!tokens.empty()) {
                                std::string dataString = tokens.front();
                                dataFromString(dataString, data);
                            }
                            emulator->send(command, data);
                        }
                    }
                }
                else if (what == "repeat") {
                    if (!tokens.empty()) {
                        std::string name = tokens.front();
                        tokens.pop_front();
                        if (!tokens.empty()) {
                            std::string intervalString = tokens.front();
                            tokens.pop_front();
                            uint32_t interval = 0;
                            if (std::all_of(intervalString.begin(), intervalString.end(),
                                            [](char c) { return std::isdigit(c); })) {
                                interval = stoi(intervalString, 0, 10);
                            }
                            if (!tokens.empty() && (interval != 0)) {
                                std::string commandString = tokens.front();
                                tokens.pop_front();
                                std::pair<uint8_t, uint8_t> command;
                                bool commandParsed = commandFromString(commandString, command);
                                if (commandParsed) {
                                    std::vector<uint8_t> data;
                                    if (!tokens.empty()) {
                                        std::string dataString = tokens.front();
                                        dataFromString(dataString, data);
                                    }
                                    emulator->repeat(name, command, data,
                                                     std::chrono::milliseconds(interval));
                                }
                            }
                        }
                    }
                }
                else if (what == "stoprepeat") {
                    if (!tokens.empty()) {
                        std::string name = tokens.front();
                        tokens.pop_front();
                        emulator->stoprepeat(name);
                    }
                    else {
                        emulator->stoprepeat();
                    }
                }
            }
        };
    messenger.initialize({}, {});
    messenger.setMessageHandler(EmulatorId::Command, handler);
    messenger.start();

    TerminateCpuComLogMessages();
    TerminateCommonLogMessages();
    return 0;
}
