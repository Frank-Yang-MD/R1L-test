#include <iostream>
#include <iterator>

#include "EmulatorMessage.h"
#include "Executors.h"
#include "Socket.h"
#include "messenger/Messenger.h"

using com::mitsubishielectric::ahu::common::InitializeCommonLogMessages;
using com::mitsubishielectric::ahu::common::Messenger;
using com::mitsubishielectric::ahu::common::MLOGD_SERIAL;
using com::mitsubishielectric::ahu::common::PausableSingleThreadExecutor;
using com::mitsubishielectric::ahu::common::SingleThreadExecutor;
using com::mitsubishielectric::ahu::common::Socket;
using com::mitsubishielectric::ahu::common::TerminateCommonLogMessages;

using com::mitsubishielectric::ahu::cpucom::impl::EmulatorId;
using com::mitsubishielectric::ahu::cpucom::impl::kEmulatorCLISocketName;

int main(int argc, char* argv[])
{
    int result = 0;
    InitializeCommonLogMessages();

    auto socket = std::make_unique<Socket>();
    auto inputExecutor = std::make_unique<PausableSingleThreadExecutor>();
    auto outputExecutor = std::make_unique<SingleThreadExecutor>();
    auto messenger = std::make_unique<Messenger<EmulatorId>>(
        kEmulatorCLISocketName, std::move(socket), std::move(inputExecutor),
        std::move(outputExecutor));

    bool initialized = messenger->initialize({}, {});
    bool connected = messenger->connect();

    if (initialized && connected) {
        std::string input;
        for (int i = 1; i < argc; ++i) {
            input.append(argv[i]);
            input.append(" ");
        }
        input.erase(std::prev(input.end()));
        messenger->sendMessage(EmulatorId::Command, input).wait();
    }
    else {
        std::cout << "can not connect to emulator" << std::endl;
        result = 1;
    }

    TerminateCommonLogMessages();
    return result;
}