/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMPL_MOCKPROTOCOL_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMPL_MOCKPROTOCOL_H_

#include "Protocol.h"

#include <mock/mock_IODevice.h>

#include <gmock/gmock.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

/**
 * MockProtocol
 */
class MockProtocol : public Protocol {
public:
    MockProtocol()
        : Protocol(std::make_unique<::testing::NiceMock<common::mock_IODevice>>())
    {
    }

    MOCK_METHOD1(send, bool(const std::vector<uint8_t>&));
    MOCK_METHOD1(receive, bool(std::vector<uint8_t>&));

    MOCK_CONST_METHOD0(r1, uint32_t());
    MOCK_CONST_METHOD0(r2, uint32_t());
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMPL_MOCKPROTOCOL_H_
