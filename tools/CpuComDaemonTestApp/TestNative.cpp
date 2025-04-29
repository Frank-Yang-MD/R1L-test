/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>

#include "TestNative.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

namespace {
const char* TAG = "CpuComDaemonTest";
}

using common::MLOGD_SERIAL;

enum BlockID { Word, Command, Data };

enum CommandID { MainCommand, SubCommand };

CpuComDaemonClient::CpuComDaemonClient()
    : m_libCpuCom(ICpuCom::create())
{
    m_libCpuCom->setErrorCallback(this);
}

CpuComDaemonClient::~CpuComDaemonClient() { testFarmStop(); }

void CpuComDaemonClient::subscribe(void)
{
    uint8_t m_command;
    uint8_t m_subCommand;
    std::string ret;
    std::string in;
    std::cout << "Write command(s). Format: comnd,subComnd ...):\n";
    std::getline(std::cin, in);
    std::vector<std::string> results = split(in, ' ');
    if (results.size() == 1) {
        if (parseCommand(results[0], m_command, m_subCommand)) {
            m_libCpuCom->subscribe(std::make_pair(m_command, m_subCommand), this);
            ret = "was subscribed\n";
        }
        else {
            ret = "bad command\n";
        }
    }
    else if (results.size() > 1) {
        std::list<common::CpuCommand> l;
        bool badCheck = false;
        for (auto i = results.begin(); i != results.end(); ++i) {
            if (parseCommand(*i, m_command, m_subCommand)) {
                l.push_back(std::make_pair(m_command, m_subCommand));
            }
            else {
                badCheck = true;
                break;
            }
        }
        if (badCheck) {
            ret = "bad command\n";
        }
        else {
            m_libCpuCom->subscribe(l, this);
            ret = "list was subscribed\n";
        }
    }
    else if (results.size() == 0) {
        ret = "bad command\n";
    }
    std::cout << ret;
    MLOGD_SERIAL(TAG, "%s\n", ret.c_str());
}

void CpuComDaemonClient::unsubscribe(void)
{
    std::string ret;
    uint8_t m_command;
    uint8_t m_subCommand;
    std::string in;
    std::cout << "Write command(s). Format: comnd,subComnd):\n";
    std::cin >> in;
    std::vector<std::string> results = split(in, ' ');
    if (parseCommand(results[0], m_command, m_subCommand)) {
        m_libCpuCom->unsubscribe(std::make_pair(m_command, m_subCommand), this);
        ret = "was unsubscribed\n";
    }
    else {
        ret = "bad command\n";
    }
    std::cout << ret;
    MLOGD_SERIAL(TAG, "%s\n", ret.c_str());
}

void CpuComDaemonClient::send(void)
{
    std::string ret;
    uint8_t m_command;
    uint8_t m_subCommand;
    std::string in;
    std::cout << "Write command(s). Format: comnd,subComnd):\n";
    std::getline(std::cin, in);
    if (!in.empty()) {
        std::vector<std::string> results = split(in, ' ');
        if (parseCommand(results[0], m_command, m_subCommand)) {
            std::cout << "Write data. Format: hex,hex, ...):\n";
            std::getline(std::cin, in);
            if (!in.empty()) {
                std::vector<uint8_t> resultsData;
                if (parseData(in, resultsData)) {
                    m_libCpuCom->send(std::make_pair(m_command, m_subCommand), resultsData);
                    ret = "was sent\n";
                }
                else {
                    ret = "bad data\n";
                }
            }
            else {
                ret = "bad data\n";
            }
        }
        else {
            ret = "bad command\n";
        }
    }
    else {
        ret = "bad command\n";
    }
    std::cout << ret;
    MLOGD_SERIAL(TAG, "%s\n", ret.c_str());
}

void CpuComDaemonClient::file(void)
{
    std::string ret;
    readFile();
    std::cout << ret;
    MLOGD_SERIAL(TAG, "%s\n", ret.c_str());
}

void CpuComDaemonClient::testFarmStart(void)
{
    std::string ret;
    std::string in;
    std::cout << "Enter interval in milliseconds:\n";
    uint32_t interval = 0;
    std::getline(std::cin, in);
    if (!in.empty()) {
        if (std::all_of(in.begin(), in.end(), ::isdigit)) {
            interval = std::stoi(in, nullptr, 10);
            common::CpuCommand subscribeCommand = std::make_pair(0xfd, 0x81);
            common::CpuCommand command = std::make_pair(0xfd, 0x01);
            std::vector<uint8_t> data = {0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
                                         0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf};
            m_libCpuCom->subscribe(subscribeCommand, this);
            repeat(command, data, std::chrono::milliseconds(interval));
            ret = "Test farm was started";
        }
        else {
            ret = "bad interval";
        }
    }
    std::cout << ret << std::endl;
    MLOGD_SERIAL(TAG, "[CpuComDaemonClient]: - %s, interval: %d", ret.c_str(), interval);
}

void CpuComDaemonClient::testFarmStop(void)
{
    std::string ret = "Test farm was stoped";
    common::CpuCommand unsubscribeCommand = std::make_pair(0xfd, 0x81);
    common::CpuCommand command = std::make_pair(0xfd, 0x01);
    m_libCpuCom->unsubscribe(unsubscribeCommand, this);
    stoprepeat(command);
    std::cout << ret << std::endl;
    MLOGD_SERIAL(TAG, "[CpuComDaemonClient]: - %s", ret.c_str());
}

void CpuComDaemonClient::cmndLineMod(void)
{
    std::string in;
    std::string ret;
    bool stopFlag = false;
    while (!stopFlag) {
        std::getline(std::cin, in);
        if (!in.empty()) {
            if (in == "file") {
                ret = readFile();
            }
            else if (in == "exit") {
                stopFlag = true;
                ret = "exit";
            }
            else {
                ret = parseGotSource(in);
            }
            std::cout << ret;
            MLOGD_SERIAL(TAG, "%s\n", ret.c_str());
        }
    }
}

std::string CpuComDaemonClient::parseGotSource(const std::string& in)
{
    std::string ret;
    uint8_t m_command;
    uint8_t m_subCommand;
    std::vector<std::string> results = split(in, ' ');
    std::string m_wordCommand = results[Word];

    if (m_wordCommand == "sendextlen") {
        std::vector<uint8_t> data;
        data.resize(512, 0x17);
        m_libCpuCom->send(std::make_pair(0xfd, 0x01), data);
        return "done\n";
    }
    if (m_wordCommand == "sendextlenfd") {
        std::vector<uint8_t> data;
        data.resize(2048, 0x17);
        m_libCpuCom->send(std::make_pair(0xfd, 0x01), data);
        return "done\n";
    }

    if ((results.size() > 1)) {
        if (m_wordCommand == "subscribe") {
            if (parseCommand(results[Command], m_command, m_subCommand)) {
                m_libCpuCom->subscribe(std::make_pair(m_command, m_subCommand), this);
                ret = "was subscribed\n";
            }
            else {
                ret = "bad command\n";
            }
        }
        else if (m_wordCommand == "unsubscribe") {
            if (parseCommand(results[Command], m_command, m_subCommand)) {
                m_libCpuCom->unsubscribe(std::make_pair(m_command, m_subCommand), this);
                ret = "was unsubscribed\n";
            }
            else {
                ret = "bad command\n";
            }
        }
        else if (m_wordCommand == "subscribelist") {
            std::list<common::CpuCommand> l;
            bool badCheck = false;
            auto i = results.begin();
            ++i;
            for (; i != results.end(); ++i) {
                std::vector<std::string> raw_comand = split(*i, ',');
                if (parseCommand(*i, m_command, m_subCommand)) {
                    l.push_back(std::make_pair(m_command, m_subCommand));
                }
                else {
                    badCheck = true;
                    break;
                }
            }
            if (badCheck) {
                ret = "bad command\n";
            }
            else {
                m_libCpuCom->subscribe(l, this);
                ret = "list was subscribed\n";
            }
        }
        else if (m_wordCommand == "send") {
            if (parseCommand(results[Command], m_command, m_subCommand)) {
                std::vector<uint8_t> resultsData;
                if (parseData(results[Data], resultsData)) {
                    m_libCpuCom->send(std::make_pair(m_command, m_subCommand), resultsData);
                    ret = "was sent\n";
                }
                else {
                    ret = "bad data\n";
                }
            }
            else {
                ret = "bad command\n";
            }
        }
        else {
            ret = "bad command\n";
        }
    }
    else {
        ret = "bad command\n";
    }
    return ret;
}

bool CpuComDaemonClient::parseData(const std::string& str, std::vector<uint8_t>& resultsData)
{
    bool result = true;
    std::vector<std::string> raw_data = split(str, ',');
    int ii = 0;
    for (auto i = raw_data.begin(); i != raw_data.end(); ++i) {
        if (isXdigits(*i)) {
            resultsData.push_back(std::stoi(*i, nullptr, 16));
            std::cout << std::hex << (int)resultsData[ii] << ' ';
            ii++;
        }
        else {
            result = false;
            break;
        }
    }
    return result;
}

std::string CpuComDaemonClient::readFile(void)
{
    std::string ret;
    std::string file_name;
    std::cout << "Write name of file: ";
    std::cin >> file_name;
    std::cout << file_name << std::endl;
    std::ifstream configfile(file_name);
    if (!configfile) {
        ret = "Could not open file to read.\n";
        std::cout << "Could not open file to read.\n";
    }
    else {
        std::string line;
        while (getline(configfile, line)) {
            std::cout << parseGotSource(line);
        }
        ret = "file has readed.\n";
    }
    return ret;
}

bool CpuComDaemonClient::parseCommand(const std::string& str, uint8_t& command, uint8_t& subCommand)
{
    std::cout << "parseCommand\n";
    bool result;
    std::vector<std::string> raw_comand = split(str, ',');
    if (check(raw_comand[MainCommand], raw_comand[SubCommand])) {
        command = std::stoi(raw_comand[MainCommand], nullptr, 16);
        subCommand = std::stoi(raw_comand[SubCommand], nullptr, 16);
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)command << ' ';
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)subCommand << ' ';
        result = true;
    }
    else {
        result = false;
    }
    return result;
}

bool CpuComDaemonClient::isXdigits(const std::string& str)
{
    return std::all_of(str.begin(), str.end(), ::isxdigit);
}

void CpuComDaemonClient::repeat(const common::CpuCommand& command,
                                const std::vector<uint8_t>& data,
                                std::chrono::milliseconds interval)
{
    if (m_repeaters.find(command) == m_repeaters.end()) {
        m_repeaters.emplace(command, std::unique_ptr<impl::Repeater>(new impl::Repeater(
                                         std::bind(&ICpuCom::send, m_libCpuCom, command, data),
                                         std::chrono::milliseconds(interval))));
        m_repeaters[command]->start();
    }
    else {
        MLOGD_SERIAL(TAG, "[CpuComDaemonClient]: Was repeat on earlier");
    }
}

void CpuComDaemonClient::stoprepeat(const common::CpuCommand& command)
{
    if (m_repeaters.find(command) != m_repeaters.end()) {
        m_repeaters[command]->stop();
        m_repeaters.erase(command);
    }
    else {
        MLOGD_SERIAL(TAG, "[CpuComDaemonClient]: Did not repeating");
    }
}

bool CpuComDaemonClient::check(const std::string& cmnd, const std::string& subCmnd)
{
    bool ret = true;
    if (isXdigits(cmnd)) {
        if (!isXdigits(subCmnd)) {
            ret = false;
        }
    }
    else {
        ret = false;
    }
    return ret;
}

std::vector<std::string> CpuComDaemonClient::split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void CpuComDaemonClient::onReceiveCommand(const common::CpuCommand& command,
                                          const std::vector<uint8_t>& payload)
{
    std::cout << "-> [" << std::hex << (int)command.first << ',' << std::hex << (int)command.second
              << "]. ";
    std::cout << "data size = " << std::dec << payload.size() << ". [";
    for (auto i = payload.begin(); i != payload.end(); ++i) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)*i;
        if (i != std::prev(payload.end(), 1)) {
            std::cout << ' ';
        }
    }
    std::cout << "]" << std::endl;
}

void CpuComDaemonClient::onError(const common::CpuCommand& command, int errorCode)
{
    (void)command;
    (void)errorCode;
    MLOGD_SERIAL(TAG, "Cmnd: 0x%2x SubCmnd: 0x%2x Error: %d\n", command.first, command.second,
                 errorCode);
}

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
