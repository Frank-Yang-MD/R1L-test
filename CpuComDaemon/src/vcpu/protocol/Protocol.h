/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_UART_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_UART_H_

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include "CpuCommand.h"
#include "StateMachine.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace common {
class IODevice;
}
namespace cpucom {
namespace impl {

const char* const kUartDeviceName = "/dev/ttySC7";
const char* const kVCPUEmulatorSocketName = "emulator";

using common::FiniteStateMachine;

enum class Event;
class SendContext;
class RecvContext;

// Data transmission control codes
enum ControlCode {
    STX = 0x02,
    ETX = 0x03,
    ENQ = 0x05,
    ACK = 0x06,
    NAK = 0x15,
    EXT_LEN = 0xfe,
};

class Protocol {
public:
    explicit Protocol(std::unique_ptr<common::IODevice> device);
    virtual ~Protocol();

public:
    virtual bool send(const std::vector<uint8_t>& data);
    virtual bool receive(std::vector<uint8_t>& data);

    uint32_t r1() const;
    uint32_t r2() const;

private:
    // send states
    Event sendIdle(SendContext& context);
    Event sendReenquiry(SendContext& context);
    Event sendEnquiry(SendContext& context);
    Event sendAcknowledgement(SendContext& context);
    Event sendFrame(SendContext& context);
    Event sendAcknowledgement2(SendContext& context);
    Event sendRetry(SendContext& context);
    Event sendNak(SendContext& context);
    Event sendWait(SendContext& context);
    Event sendDone(SendContext& context);
    Event sendError(SendContext& context);
    Event sendEnd(SendContext& context);

    // recv states
    Event recvControlCode(uint8_t code, RecvContext& context);
    Event recvIdle(RecvContext& context);
    Event recvPoll(RecvContext& context);
    Event recvRepoll(RecvContext& context);
    Event recvEnquiry(RecvContext& context);
    Event recvAcknowledgement(RecvContext& context);
    Event recvLength(RecvContext& context);
    Event recvDataCommand(RecvContext& context);
    Event recvDataExtLen(RecvContext& context);
    Event recvDataFrameNumber(RecvContext& context);
    Event recvData(RecvContext& context);
    Event recvChecksum(RecvContext& context);
    Event recvAcknowledgement2(RecvContext& context);
    Event recvNak(RecvContext& context);
    Event recvRetry(RecvContext& context);
    Event recvDone(RecvContext& context);
    Event recvError(RecvContext& context);
    Event recvEnd(RecvContext& context);

private:
    std::unique_ptr<common::IODevice> m_device;
    std::mutex m_accessLock;
    std::unique_ptr<FiniteStateMachine<Event, SendContext>> m_sendMachine;
    std::unique_ptr<FiniteStateMachine<Event, RecvContext>> m_recvMachine;
    uint32_t m_r1;
    uint32_t m_r2;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_UART_H_
