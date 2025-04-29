#include "Utils.h"

#include <sstream>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace utils {

std::deque<std::string> tokenize(const std::string& s, std::string::value_type delimiter)
{
    std::istringstream ss(s);
    std::deque<std::string> tokens;
    for (std::string i; std::getline(ss, i, delimiter);) {
        tokens.push_back(i);
    }
    return tokens;
}

bool commandFromString(const std::string& s, common::CpuCommand& command)
{
    std::istringstream ss(s);
    std::string cmd;
    std::getline(ss, cmd, ',');
    std::string subcmd;
    std::getline(ss, subcmd, ',');

    bool valid = !cmd.empty() && !subcmd.empty();
    if (valid) {
        bool b1 = std::all_of(cmd.begin(), cmd.end(), [](auto a) { return std::isxdigit(a); });
        bool b2 =
            std::all_of(subcmd.begin(), subcmd.end(), [](auto a) { return std::isxdigit(a); });
        bool valid = b1 && b2;
        if (valid) {
            command = std::make_pair(std::stoi(cmd, 0, 16), std::stoi(subcmd, 0, 16));
        }
    }
    return valid;
}

bool dataFromString(const std::string& s, std::vector<uint8_t>& data)
{
    std::istringstream ss(s);
    bool result = true;
    for (std::string d; std::getline(ss, d, ',');) {
        if (!d.empty() &&
            std::all_of(d.begin(), d.end(), [](auto a) { return std::isxdigit(a); })) {
            data.push_back(std::stoi(d, 0, 16));
        }
        else {
            data.clear();
            result = false;
            break;
        }
    }
    return result;
}

}  // namespace utils
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com