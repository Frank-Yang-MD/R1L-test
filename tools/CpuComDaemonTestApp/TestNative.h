/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_TESTNATIVE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_TESTNATIVE_H_

#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "CpuCom.h"
#include "Log.h"
#include "Repeater.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

class CpuComDaemonClient
    : public ICpuCommandListener
    , public ICpuCommandErrorListener {
public:
    CpuComDaemonClient();
    ~CpuComDaemonClient();
    void subscribe(void);
    void unsubscribe(void);
    void send(void);
    void file(void);
    void testFarmStart(void);
    void testFarmStop(void);
    void cmndLineMod(void);

private:
    bool parseData(const std::string& str, std::vector<uint8_t>& resultsData);
    bool parseCommand(const std::string& str, uint8_t& command, uint8_t& subCommand);
    bool check(const std::string& cmnd, const std::string& subCmnd);
    void repeat(const common::CpuCommand& command,
                const std::vector<uint8_t>& data,
                std::chrono::milliseconds interval);
    void stoprepeat(const common::CpuCommand& command);
    bool isXdigits(const std::string& str);
    std::string readFile(void);
    std::string parseGotSource(const std::string& in);
    std::vector<std::string> split(const std::string& s, char delimiter);

private:
    void onReceiveCommand(const common::CpuCommand& command, const std::vector<uint8_t>& data);
    void onError(const common::CpuCommand& command, int errorCode);

private:
    std::shared_ptr<ICpuCom> m_libCpuCom;
    std::map<common::CpuCommand, std::unique_ptr<impl::Repeater>> m_repeaters;
};

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_TESTNATIVE_H_
