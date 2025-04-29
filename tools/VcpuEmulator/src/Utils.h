/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_UTILS_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_UTILS_H_

#include "CpuCommand.h"
#include <deque>
#include <string>
#include <vector>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace utils {

std::deque<std::string> tokenize(const std::string& s, std::string::value_type delimiter);
bool commandFromString(const std::string& s, common::CpuCommand& command);
bool dataFromString(const std::string& s, std::vector<uint8_t>& data);

}  // namespace utils
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_UTILS_H_
