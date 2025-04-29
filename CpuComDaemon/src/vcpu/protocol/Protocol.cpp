/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "Protocol.h"

#include <poll.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <thread>

#include "Checksum.h"
#include "CpuComDaemonLog.h"
#include "IODevice.h"
#include "Log.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

namespace {

// Lengths of the message fields
const int32_t kStxLength = 1;
const int32_t kLenLength = 1;
const int32_t kCmdLength = 3;
const int32_t kExtLenLength = 3;
const int32_t kFrameNumberLength = 2;
const int32_t kTotalFramesLength = 2;
const int32_t kEtxLength = 1;
const int32_t kCsLength = 1;
const int32_t kFrameHeaderLength = kStxLength + kLenLength;
const int32_t kFrameFooterLength = kEtxLength + kCsLength;

// Frame length limitations
const int32_t kMinFrameLength = 5;
const int32_t kMaxFrameLength = 253;
const int32_t kMaxActualDataLength = 1024;
const int32_t kMaxExtendedLengthFrameLength = 1032;
const int32_t kMaxExtendedLengthFrameDivisionFrameLength = 1036;

// Timeouts
// const std::chrono::milliseconds kTimeout1 = std::chrono::milliseconds(3);
// const std::chrono::milliseconds kTimeout2 = std::chrono::milliseconds(3);
const std::chrono::milliseconds kTimeout3 = std::chrono::milliseconds(50);
const std::chrono::milliseconds kTimeout4 = std::chrono::milliseconds(50);
const std::chrono::milliseconds kTimeout5 = std::chrono::milliseconds(5);
// const std::chrono::milliseconds kTimeout6 = std::chrono::milliseconds(30);

// Error handling settings
const int32_t kMaxNumberOfSendAttempts = 240;
const int32_t kMaxNumberOfRecvAttempts = 5;

const int32_t kSendAttemptsForPortReinit = 6;

std::vector<std::vector<uint8_t>> prepareFrames(const std::vector<uint8_t>& d)
{
    std::vector<uint8_t> data = d;
    std::vector<std::vector<uint8_t>> frames;
    if (data.size() <= kMaxFrameLength - kFrameFooterLength) {
        // regular frame
        std::vector<uint8_t> frame;
        frame.reserve(kFrameHeaderLength + kMaxFrameLength);
        frame.push_back(STX);
        frame.push_back(data.size() + kFrameFooterLength);
        frame.insert(frame.end(), data.begin(), data.end());
        frame.push_back(ETX);
        // calculate checksum for [LEN]..[ETX] range
        uint8_t cs = common::checksum(std::next(frame.begin()), frame.end());
        frame.push_back(cs);
        frames.push_back(frame);
    }
    else if (data.size() <= (kMaxExtendedLengthFrameLength - kFrameFooterLength)) {
        // extended length, no frame division
        std::vector<uint8_t> frame;
        frame.reserve(kFrameHeaderLength + kMaxExtendedLengthFrameLength);
        frame.push_back(STX);
        frame.push_back(EXT_LEN);
        frame.push_back(data.at(0));  // command
        frame.push_back(data.at(1));  // sub command
        frame.push_back(data.at(2));  // codebit

        uint32_t length = data.size() + kExtLenLength + kFrameFooterLength;
        uint8_t length1 = (length >> 16) & 0x000000ff;
        uint8_t length2 = (length >> 8) & 0x000000ff;
        uint8_t length3 = length & 0x000000ff;
        frame.push_back(length1);
        frame.push_back(length2);
        frame.push_back(length3);
        frame.insert(frame.end(), std::next(data.begin(), 3), data.end());
        frame.push_back(ETX);
        // calculate checksum for [LEN]..[ETX] range
        uint8_t cs = common::checksum(std::next(frame.begin()), frame.end());
        frame.push_back(cs);
        frames.push_back(frame);
    }
    else {
        // extended length, frame division enabled
        uint32_t actualDataLength = data.size() - kCmdLength;
        uint32_t framesCount = (actualDataLength / kMaxActualDataLength) +
                               ((actualDataLength % kMaxActualDataLength) > 0);
        assert(framesCount > 1);

        uint8_t command = data.at(0);
        uint8_t subCommand = data.at(1);
        uint8_t codebit = data.at(2);

        for (uint32_t i = 0; i < framesCount; ++i) {
            std::vector<uint8_t> frame;
            frame.reserve(kFrameHeaderLength + kMaxExtendedLengthFrameDivisionFrameLength);
            frame.push_back(STX);
            frame.push_back(EXT_LEN);
            frame.push_back(command);
            frame.push_back(subCommand);
            if (i == framesCount - 1) {
                codebit |= 0x03;  // split, last frame
            }
            else {
                codebit |= 0x02;  // split, next frame present
            }
            frame.push_back(codebit);

            uint32_t length = std::min<uint32_t>(actualDataLength, kMaxActualDataLength);
            actualDataLength -= length;
            length += kCmdLength + kExtLenLength + kFrameNumberLength + kTotalFramesLength +
                      kFrameFooterLength;
            uint8_t length1 = (length >> 16) & 0x000000ff;
            uint8_t length2 = (length >> 8) & 0x000000ff;
            uint8_t length3 = length & 0x000000ff;
            frame.push_back(length1);
            frame.push_back(length2);
            frame.push_back(length3);
            uint8_t frameNumber1 = (i >> 8) & 0x000000ff;
            uint8_t frameNumber2 = i & 0x000000ff;
            frame.push_back(frameNumber1);
            frame.push_back(frameNumber2);
            uint8_t framesCount1 = (framesCount >> 8) & 0x000000ff;
            uint8_t framesCount2 = framesCount & 0x000000ff;
            frame.push_back(framesCount1);
            frame.push_back(framesCount2);
            auto begin = (i == 0) ? std::next(data.begin(), 3) : data.begin();
            const uint32_t dataLength = std::min<uint32_t>(kMaxActualDataLength, data.size());
            frame.insert(frame.end(), begin, std::next(begin, dataLength));
            frame.push_back(ETX);
            // calculate checksum for [LEN]..[ETX] range
            uint8_t cs = common::checksum(std::next(frame.begin()), frame.end());
            frame.push_back(cs);
            frames.push_back(frame);
            data.erase(data.begin(),
                       std::next(data.begin(), dataLength + ((i == 0) ? kCmdLength : 0)));
        }
        assert(actualDataLength == 0);
    }
    return frames;
}
}  // namespace

enum class Event { Pass, Busy, Wait, Deny, Fail };

using common::IODevice;
using common::MLOGD;
using common::MLOGV;
using common::MLOGW;

class Context {
public:
    explicit Context()
        : m_currentFrame(0)
        , m_result(false)
    {
    }

    virtual ~Context() = default;

public:
    std::vector<uint8_t>& currentFrame() { return m_frames.at(m_currentFrame); }
    bool result() const { return m_result; }
    void setResult(bool result) { m_result = result; }

    const std::vector<std::vector<uint8_t>>& frames() const { return m_frames; }

    virtual void onFrameCompleted()
    {
        ++m_currentFrame;
        assert(m_currentFrame < m_frames.size());
    }

protected:
    uint32_t m_currentFrame;
    std::vector<std::vector<uint8_t>> m_frames;
    bool m_result;
};

class SendContext : public Context {
public:
    explicit SendContext(const std::vector<uint8_t>& data)
        : Context()
    {
        m_frames = prepareFrames(data);
    }

    virtual ~SendContext() = default;

public:
    bool hasFramesToSend() const { return m_currentFrame < (m_frames.size() - 1); }
};

class RecvContext : public Context {
public:
    enum class TransmitionType {
        NotDefinedYet,
        Regular,
        ExtendedLength,
        ExtendedLengthWithFrameDivision,
    };

    explicit RecvContext()
        : Context()
        , m_transmitionType(TransmitionType::NotDefinedYet)
        , m_dataLength(0)
        , m_frameNumber(0)
        , m_totalFrames(0)
    {
        m_frames.push_back(std::vector<uint8_t>());
        m_frames.front().reserve(kFrameHeaderLength + kMaxExtendedLengthFrameDivisionFrameLength);
    }

    virtual ~RecvContext() = default;

public:
    virtual void onFrameCompleted() override
    {
        m_frames.push_back(std::vector<uint8_t>());
        m_frames.back().reserve(kFrameHeaderLength + kMaxExtendedLengthFrameDivisionFrameLength);
        Context::onFrameCompleted();
    }

    void setTransmitionType(TransmitionType type) { m_transmitionType = type; }
    TransmitionType transmitionType() const { return m_transmitionType; }

    void setDataLength(uint32_t length) { m_dataLength = length; }
    uint32_t getDataLength() const { return m_dataLength; }
    void setTotalFrames(uint32_t value) { m_totalFrames = value; }
    uint32_t totalFrames() const { return m_totalFrames; }
    void setFrameNumber(uint32_t value) { m_frameNumber = value; }
    uint32_t frameNumber() const { return m_frameNumber; }

private:
    TransmitionType m_transmitionType;
    uint32_t m_dataLength;  // data length when transmition type == Regular
    // frame number and total frames count when transmition type == ExtendedLengthWithFrameDivision
    uint32_t m_frameNumber;
    uint32_t m_totalFrames;
};

Protocol::Protocol(std::unique_ptr<IODevice> device)
    : m_device(std::move(device))
    , m_sendMachine(new FiniteStateMachine<Event, SendContext>())
    , m_recvMachine(new FiniteStateMachine<Event, RecvContext>())
    , m_r1(0)
    , m_r2(0)
{
    // this is (almost) a copy of a state machine implemented in syscon_uart.c
    // the only differences are:
    // 1. the 'end' state, which is used as a final state,
    // 'done' and 'error'(which are the final states in syscon_uart.c) pass on to the 'end' state.
    // 2. done ---WAIT--> idle transitions which are used to send/receive extended length messages
    // with frame division enabled
    m_sendMachine->addState("idle", std::bind(&Protocol::sendIdle, this, std::placeholders::_1));
    m_sendMachine->addState("enquiry",
                            std::bind(&Protocol::sendEnquiry, this, std::placeholders::_1));
    m_sendMachine->addState("reenquiry",
                            std::bind(&Protocol::sendReenquiry, this, std::placeholders::_1));
    m_sendMachine->addState("ack",
                            std::bind(&Protocol::sendAcknowledgement, this, std::placeholders::_1));
    m_sendMachine->addState("frame", std::bind(&Protocol::sendFrame, this, std::placeholders::_1));
    m_sendMachine->addState(
        "ack2", std::bind(&Protocol::sendAcknowledgement2, this, std::placeholders::_1));
    m_sendMachine->addState("retry", std::bind(&Protocol::sendRetry, this, std::placeholders::_1));
    m_sendMachine->addState("nak", std::bind(&Protocol::sendNak, this, std::placeholders::_1));
    m_sendMachine->addState("wait", std::bind(&Protocol::sendWait, this, std::placeholders::_1));
    m_sendMachine->addState("done", std::bind(&Protocol::sendDone, this, std::placeholders::_1));
    m_sendMachine->addState("error", std::bind(&Protocol::sendError, this, std::placeholders::_1));
    m_sendMachine->addState("end", std::bind(&Protocol::sendEnd, this, std::placeholders::_1));

    m_sendMachine->setInitialState("idle");
    m_sendMachine->setFinalState("end");

    m_sendMachine->addTransition("idle", Event::Pass, "enquiry");
    m_sendMachine->addTransition("idle", Event::Busy, "retry");
    m_sendMachine->addTransition("idle", Event::Wait, "retry");
    m_sendMachine->addTransition("idle", Event::Deny, "retry");
    m_sendMachine->addTransition("idle", Event::Fail, "retry");

    m_sendMachine->addTransition("enquiry", Event::Pass, "reenquiry");
    m_sendMachine->addTransition("enquiry", Event::Busy, "retry");
    m_sendMachine->addTransition("enquiry", Event::Wait, "retry");
    m_sendMachine->addTransition("enquiry", Event::Deny, "retry");
    m_sendMachine->addTransition("enquiry", Event::Fail, "retry");

    m_sendMachine->addTransition("reenquiry", Event::Pass, "ack");
    m_sendMachine->addTransition("reenquiry", Event::Busy, "retry");
    m_sendMachine->addTransition("reenquiry", Event::Wait, "nak");
    m_sendMachine->addTransition("reenquiry", Event::Deny, "retry");
    m_sendMachine->addTransition("reenquiry", Event::Fail, "retry");

    m_sendMachine->addTransition("ack", Event::Pass, "frame");
    m_sendMachine->addTransition("ack", Event::Busy, "reenquiry");
    m_sendMachine->addTransition("ack", Event::Wait, "nak");
    m_sendMachine->addTransition("ack", Event::Deny, "retry");
    m_sendMachine->addTransition("ack", Event::Fail, "retry");

    m_sendMachine->addTransition("frame", Event::Pass, "ack2");
    m_sendMachine->addTransition("frame", Event::Busy, "retry");
    m_sendMachine->addTransition("frame", Event::Wait, "retry");
    m_sendMachine->addTransition("frame", Event::Deny, "retry");
    m_sendMachine->addTransition("frame", Event::Fail, "retry");

    m_sendMachine->addTransition("ack2", Event::Pass, "done");
    m_sendMachine->addTransition("ack2", Event::Busy, "retry");
    m_sendMachine->addTransition("ack2", Event::Wait, "retry");
    m_sendMachine->addTransition("ack2", Event::Deny, "retry");
    m_sendMachine->addTransition("ack2", Event::Fail, "retry");

    m_sendMachine->addTransition("retry", Event::Pass, "enquiry");
    m_sendMachine->addTransition("retry", Event::Busy, "retry");
    m_sendMachine->addTransition("retry", Event::Wait, "retry");
    m_sendMachine->addTransition("retry", Event::Deny, "retry");
    m_sendMachine->addTransition("retry", Event::Fail, "error");

    m_sendMachine->addTransition("nak", Event::Pass, "wait");
    m_sendMachine->addTransition("nak", Event::Busy, "retry");
    m_sendMachine->addTransition("nak", Event::Wait, "retry");
    m_sendMachine->addTransition("nak", Event::Deny, "retry");
    m_sendMachine->addTransition("nak", Event::Fail, "retry");

    m_sendMachine->addTransition("wait", Event::Pass, "enquiry");
    m_sendMachine->addTransition("wait", Event::Busy, "retry");
    m_sendMachine->addTransition("wait", Event::Wait, "retry");
    m_sendMachine->addTransition("wait", Event::Deny, "retry");
    m_sendMachine->addTransition("wait", Event::Fail, "retry");

    m_sendMachine->addTransition("done", Event::Pass, "end");
    m_sendMachine->addTransition("done", Event::Busy, "end");
    m_sendMachine->addTransition("done", Event::Wait,
                                 "reenquiry");  // this transition is used to send extended length
                                                // messages with frame divisions
    m_sendMachine->addTransition("done", Event::Deny, "end");
    m_sendMachine->addTransition("done", Event::Fail, "end");

    m_sendMachine->addTransition("error", Event::Pass, "end");
    m_sendMachine->addTransition("error", Event::Busy, "end");
    m_sendMachine->addTransition("error", Event::Wait, "end");
    m_sendMachine->addTransition("error", Event::Deny, "end");
    m_sendMachine->addTransition("error", Event::Fail, "end");

    m_recvMachine->addState("idle", std::bind(&Protocol::recvIdle, this, std::placeholders::_1));
    m_recvMachine->addState("poll", std::bind(&Protocol::recvPoll, this, std::placeholders::_1));
    m_recvMachine->addState("repoll",
                            std::bind(&Protocol::recvRepoll, this, std::placeholders::_1));
    m_recvMachine->addState("enquiry",
                            std::bind(&Protocol::recvEnquiry, this, std::placeholders::_1));
    m_recvMachine->addState("ack",
                            std::bind(&Protocol::recvAcknowledgement, this, std::placeholders::_1));
    m_recvMachine->addState(
        "stx", std::bind(&Protocol::recvControlCode, this, STX, std::placeholders::_1));
    m_recvMachine->addState("len", std::bind(&Protocol::recvLength, this, std::placeholders::_1));
    m_recvMachine->addState("datacmd",
                            std::bind(&Protocol::recvDataCommand, this, std::placeholders::_1));
    m_recvMachine->addState("dataextlen",
                            std::bind(&Protocol::recvDataExtLen, this, std::placeholders::_1));
    m_recvMachine->addState("dataframenumber",
                            std::bind(&Protocol::recvDataFrameNumber, this, std::placeholders::_1));
    m_recvMachine->addState("data", std::bind(&Protocol::recvData, this, std::placeholders::_1));
    m_recvMachine->addState(
        "etx", std::bind(&Protocol::recvControlCode, this, ETX, std::placeholders::_1));
    m_recvMachine->addState("cs", std::bind(&Protocol::recvChecksum, this, std::placeholders::_1));
    m_recvMachine->addState(
        "ack2", std::bind(&Protocol::recvAcknowledgement2, this, std::placeholders::_1));
    m_recvMachine->addState("nak", std::bind(&Protocol::recvNak, this, std::placeholders::_1));
    m_recvMachine->addState("retry", std::bind(&Protocol::recvRetry, this, std::placeholders::_1));
    m_recvMachine->addState("done", std::bind(&Protocol::recvDone, this, std::placeholders::_1));
    m_recvMachine->addState("error", std::bind(&Protocol::recvError, this, std::placeholders::_1));
    m_recvMachine->addState("end", std::bind(&Protocol::recvEnd, this, std::placeholders::_1));

    m_recvMachine->setInitialState("idle");
    m_recvMachine->setFinalState("end");

    m_recvMachine->addTransition("idle", Event::Pass, "poll");
    m_recvMachine->addTransition("idle", Event::Busy, "error");
    m_recvMachine->addTransition("idle", Event::Wait, "error");
    m_recvMachine->addTransition("idle", Event::Deny, "error");
    m_recvMachine->addTransition("idle", Event::Fail, "error");

    m_recvMachine->addTransition("poll", Event::Pass, "enquiry");
    m_recvMachine->addTransition("poll", Event::Busy, "error");
    m_recvMachine->addTransition("poll", Event::Wait, "error");
    m_recvMachine->addTransition("poll", Event::Deny, "error");
    m_recvMachine->addTransition("poll", Event::Fail, "error");

    m_recvMachine->addTransition("repoll", Event::Pass, "poll");
    m_recvMachine->addTransition("repoll", Event::Busy, "error");
    m_recvMachine->addTransition("repoll", Event::Wait, "error");
    m_recvMachine->addTransition("repoll", Event::Deny, "error");
    m_recvMachine->addTransition("repoll", Event::Fail, "error");

    m_recvMachine->addTransition("enquiry", Event::Pass, "ack");
    m_recvMachine->addTransition("enquiry", Event::Busy, "repoll");
    m_recvMachine->addTransition("enquiry", Event::Wait, "error");
    m_recvMachine->addTransition("enquiry", Event::Deny, "retry");
    m_recvMachine->addTransition("enquiry", Event::Fail, "error");

    m_recvMachine->addTransition("ack", Event::Pass, "stx");
    m_recvMachine->addTransition("ack", Event::Busy, "error");
    m_recvMachine->addTransition("ack", Event::Wait, "error");
    m_recvMachine->addTransition("ack", Event::Deny, "error");
    m_recvMachine->addTransition("ack", Event::Fail, "error");

    m_recvMachine->addTransition("stx", Event::Pass, "len");
    m_recvMachine->addTransition("stx", Event::Busy, "error");
    m_recvMachine->addTransition("stx", Event::Wait, "error");
    m_recvMachine->addTransition("stx", Event::Deny, "nak");
    m_recvMachine->addTransition("stx", Event::Fail, "error");

    m_recvMachine->addTransition("len", Event::Pass, "datacmd");
    m_recvMachine->addTransition("len", Event::Busy, "error");
    m_recvMachine->addTransition("len", Event::Wait, "error");
    m_recvMachine->addTransition("len", Event::Deny, "nak");
    m_recvMachine->addTransition("len", Event::Fail, "error");

    m_recvMachine->addTransition("datacmd", Event::Pass, "dataextlen");
    m_recvMachine->addTransition("datacmd", Event::Busy, "error");
    m_recvMachine->addTransition("datacmd", Event::Wait, "error");
    m_recvMachine->addTransition("datacmd", Event::Deny, "nak");
    m_recvMachine->addTransition("datacmd", Event::Fail, "error");

    m_recvMachine->addTransition("dataextlen", Event::Pass, "dataframenumber");
    m_recvMachine->addTransition("dataextlen", Event::Busy, "error");
    m_recvMachine->addTransition("dataextlen", Event::Wait, "error");
    m_recvMachine->addTransition("dataextlen", Event::Deny, "nak");
    m_recvMachine->addTransition("dataextlen", Event::Fail, "error");

    m_recvMachine->addTransition("dataframenumber", Event::Pass, "data");
    m_recvMachine->addTransition("dataframenumber", Event::Busy, "error");
    m_recvMachine->addTransition("dataframenumber", Event::Wait, "error");
    m_recvMachine->addTransition("dataframenumber", Event::Deny, "nak");
    m_recvMachine->addTransition("dataframenumber", Event::Fail, "error");

    m_recvMachine->addTransition("data", Event::Pass, "etx");
    m_recvMachine->addTransition("data", Event::Busy, "error");
    m_recvMachine->addTransition("data", Event::Wait, "error");
    m_recvMachine->addTransition("data", Event::Deny, "nak");
    m_recvMachine->addTransition("data", Event::Fail, "error");

    m_recvMachine->addTransition("etx", Event::Pass, "cs");
    m_recvMachine->addTransition("etx", Event::Busy, "error");
    m_recvMachine->addTransition("etx", Event::Wait, "error");
    m_recvMachine->addTransition("etx", Event::Deny, "nak");
    m_recvMachine->addTransition("etx", Event::Fail, "error");

    m_recvMachine->addTransition("cs", Event::Pass, "ack2");
    m_recvMachine->addTransition("cs", Event::Busy, "error");
    m_recvMachine->addTransition("cs", Event::Wait, "error");
    m_recvMachine->addTransition("cs", Event::Deny, "nak");
    m_recvMachine->addTransition("cs", Event::Fail,
                                 "nak");  // in syscon_uart.c cs ---FAIL--> idle,
                                          // I think it's wrong, I changed it to cs ---FAIL--> nak

    m_recvMachine->addTransition("ack2", Event::Pass, "done");
    m_recvMachine->addTransition("ack2", Event::Busy, "error");
    m_recvMachine->addTransition("ack2", Event::Wait, "error");
    m_recvMachine->addTransition("ack2", Event::Deny, "error");
    m_recvMachine->addTransition("ack2", Event::Fail, "error");

    m_recvMachine->addTransition("nak", Event::Pass, "retry");
    m_recvMachine->addTransition("nak", Event::Busy, "error");
    m_recvMachine->addTransition("nak", Event::Wait, "error");
    m_recvMachine->addTransition("nak", Event::Deny, "error");
    m_recvMachine->addTransition("nak", Event::Fail, "error");

    m_recvMachine->addTransition("retry", Event::Pass, "repoll");
    m_recvMachine->addTransition("retry", Event::Busy, "error");
    m_recvMachine->addTransition("retry", Event::Wait, "error");
    m_recvMachine->addTransition("retry", Event::Deny, "error");
    m_recvMachine->addTransition("retry", Event::Fail, "error");

    m_recvMachine->addTransition("done", Event::Pass, "end");
    m_recvMachine->addTransition("done", Event::Busy, "end");
    m_recvMachine->addTransition("done", Event::Wait,
                                 "enquiry");  // this transition is used to receive extended length
                                              // messages with frame divisions
    m_recvMachine->addTransition("done", Event::Deny, "end");
    m_recvMachine->addTransition("done", Event::Fail, "end");

    m_recvMachine->addTransition("error", Event::Pass, "end");
    m_recvMachine->addTransition("error", Event::Busy, "end");
    m_recvMachine->addTransition("error", Event::Wait, "end");
    m_recvMachine->addTransition("error", Event::Deny, "end");
    m_recvMachine->addTransition("error", Event::Fail, "end");

    m_device->open(IODevice::OpenMode::ReadWrite);
}

Protocol::~Protocol() { m_device->close(); }

bool Protocol::receive(std::vector<uint8_t>& data)
{
    RecvContext context;
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ReceiveFrameBegin);
    m_recvMachine->run(context);
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ReceiveFrameEnd);
    if (context.result() == true) {
        data.reserve(kMaxActualDataLength * context.totalFrames() + kCmdLength);
        if (context.transmitionType() ==
            RecvContext::TransmitionType::ExtendedLengthWithFrameDivision) {
            for (uint32_t i = 0; i < context.totalFrames(); ++i) {
                if (i == 0) {
                    data = context.frames().at(i);
                    auto extendedLengthBegin =
                        std::next(data.begin(), kStxLength + kLenLength + kCmdLength);
                    auto totalFramesEnd =
                        std::next(extendedLengthBegin,
                                  kExtLenLength + kFrameNumberLength + kTotalFramesLength);
                    data.erase(extendedLengthBegin, totalFramesEnd);
                    data.erase(std::prev(data.end(), kCsLength + kEtxLength), data.end());
                }
                else {
                    std::vector<uint8_t> frame = context.frames().at(i);
                    auto actualDataBegin = std::next(
                        frame.begin(), kStxLength + kLenLength + kCmdLength + kExtLenLength +
                                           kFrameNumberLength + kTotalFramesLength);
                    auto actualDataEnd = std::prev(frame.end(), kCsLength + kEtxLength);
                    if (i == (context.totalFrames() - 1)) {
                        actualDataEnd = frame.end();
                    }
                    data.insert(data.end(), actualDataBegin, actualDataEnd);
                }
            }
        }
        else if (context.transmitionType() == RecvContext::TransmitionType::ExtendedLength) {
            data = context.currentFrame();
            auto extendedLengthBegin =
                std::next(data.begin(), kStxLength + kLenLength + kCmdLength);
            auto extendedLengthEnd = std::next(extendedLengthBegin, kExtLenLength);
            data.erase(extendedLengthBegin, extendedLengthEnd);
        }
        else {
            data = context.currentFrame();
        }

        // only uart Protocol knows about STX, LEN, ETX and CS.
        data.erase(data.begin(), std::next(data.begin(), kStxLength + kLenLength));
        data.erase(std::prev(data.end(), kCsLength + kEtxLength), data.end());
        return true;
    }
    else {
        return false;
    }
}

bool Protocol::send(const std::vector<uint8_t>& data)
{
    assert((data.size() > 2) && "data.size() > 2");
    SendContext context(data);
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::SendFrameBegin, data[0], data[1]);
    m_sendMachine->run(context);
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::SendFrameEnd);
    return context.result();
}

uint32_t Protocol::r1() const { return m_r1; }

uint32_t Protocol::r2() const { return m_r2; }

Event Protocol::sendIdle(SendContext&)
{
    m_r2 = 0;
    return Event::Pass;
}

Event Protocol::sendEnquiry(SendContext&)
{
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::LockDevice);
    m_accessLock.lock();
    return Event::Pass;
}

Event Protocol::sendReenquiry(SendContext&)
{
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::Enquiry);
    uint8_t data = 0;
    Event result = Event::Fail;

    IODevice::Result deviceResult = IODevice::Result::Success;
    while (deviceResult == IODevice::Result::Success) {
        deviceResult = m_device->read(&data, IODevice::kTimeoutImmediate);
    }

    if (deviceResult == IODevice::Result::Timeout) {
        IODevice::Result deviceResult = std::get<IODevice::Result>(m_device->write(ENQ));
        if (deviceResult == IODevice::Result::Success) {
            result = Event::Pass;
        }
        else {
            MLOGW(common::FunctionID::cpuc_daemon_error,
                  daemon::ErrorLogID::sendReenquiry_writeError);
            result = Event::Fail;
        }
    }
    else {
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::sendReenquiry_readError);
        result = Event::Fail;
    }
    return result;
}

Event Protocol::sendAcknowledgement(SendContext&)
{
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::WaitForACK);
    uint8_t data = 0;
    Event result = Event::Fail;
    IODevice::Result deviceResult = m_device->read(&data, kTimeout3);

    switch (deviceResult) {
    case IODevice::Result::Success:
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ByteReceived, data);
        switch (data) {
        case ACK:
            result = Event::Pass;
            break;
        case NAK:
            MLOGW(common::FunctionID::cpuc_daemon_error,
                  daemon::ErrorLogID::sendAcknowledgement_ackConflict_Busy);
            result = Event::Busy;
            break;
        case ENQ:
            MLOGW(common::FunctionID::cpuc_daemon_error,
                  daemon::ErrorLogID::sendAcknowledgement_ackConflict_Wait);
            result = Event::Wait;
            break;
        default:
            MLOGW(common::FunctionID::cpuc_daemon_error,
                  daemon::ErrorLogID::sendAcknowledgement_ackConflict_Deny, data);
            result = Event::Deny;
            break;
        }
        break;
    case IODevice::Result::Timeout:
        MLOGW(common::FunctionID::cpuc_daemon_error,
              daemon::ErrorLogID::sendAcknowledgement_timeout);
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::Timeout);
        result = Event::Deny;
        break;
    default:
        MLOGW(common::FunctionID::cpuc_daemon_error,
              daemon::ErrorLogID::sendAcknowledgement_readError);
        result = Event::Fail;
        break;
    }
    return result;
}

Event Protocol::sendFrame(SendContext& context)
{
    IODevice::Result deviceResult = IODevice::Result::Success;
    Event result = Event::Fail;
    deviceResult = std::get<IODevice::Result>(
        m_device->write(context.currentFrame().data(), context.currentFrame().size()));
    if (deviceResult == IODevice::Result::Success) {
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::FrameSent);
        result = Event::Pass;
    }
    else {
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::SendFrameFailed);
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::sendFrame_writeError);
        result = Event::Fail;
    }
    return result;
}

Event Protocol::sendAcknowledgement2(SendContext& context) { return sendAcknowledgement(context); }

Event Protocol::sendRetry(SendContext&)
{
    ++m_r2;

    Event result = Event::Fail;
    if (m_r2 < kMaxNumberOfSendAttempts) {
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::UnlockDevice);
        m_accessLock.unlock();
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::WaitT5);
        std::this_thread::sleep_for(kTimeout5);
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::Retrying);

        if ((m_r2 % kSendAttemptsForPortReinit) == 0) {
            m_device->close();
            bool result = m_device->open(IODevice::OpenMode::ReadWrite);
            MLOGE(common::FunctionID::cpuc_daemon_error,
                  daemon::ErrorLogID::SendMaxRetryCountRecovery, kSendAttemptsForPortReinit,
                  result);
        }

        result = Event::Pass;
    }
    else {
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::sendRetry_Error);
        result = Event::Fail;
    }
    return result;
}

Event Protocol::sendNak(SendContext&)
{
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::SendNAK);
    const auto result =
        std::get<IODevice::Result>(m_device->write(NAK)) == IODevice::Result::Success;

    if (!result) {
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::sendNak_writeError);
    }

    return result ? Event::Pass : Event::Fail;
}

Event Protocol::sendWait(SendContext&)
{
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::UnlockDevice);
    m_accessLock.unlock();
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::WaitT5);
    std::this_thread::sleep_for(kTimeout5);
    return Event::Pass;
}

Event Protocol::sendDone(SendContext& context)
{
    if (context.hasFramesToSend()) {
        context.onFrameCompleted();
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ProcessNextFrame);
        return Event::Wait;
    }

    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::SendDone);
    context.setResult(true);
    return Event::Pass;
}

Event Protocol::sendError(SendContext& context)
{
    MLOGE(common::FunctionID::cpuc_daemon_error,
          daemon::ErrorLogID::SendCommunicationErrorRecovery);

    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::Error);
    context.setResult(false);
    return Event::Pass;
}

Event Protocol::sendEnd(SendContext&)
{
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::UnlockDevice);
    m_accessLock.unlock();
    return Event::Pass;
}

Event Protocol::recvControlCode(uint8_t code, RecvContext& context)
{
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::WaitForControlCode,
          code == STX ? "STX" : "ETX", code);
    uint8_t b = 0;
    Event result = Event::Fail;
    IODevice::Result deviceResult = m_device->read(&b, kTimeout3);
    switch (deviceResult) {
    case IODevice::Result::Success:
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ByteReceived, b);
        if (b == code) {
            context.currentFrame().push_back(b);
            result = Event::Pass;
        }
        else {
            MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvControlCode_Deny,
                  code, b);
            result = Event::Deny;
        }
        break;
    case IODevice::Result::Timeout:
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvControlCode_Timeout);
        result = Event::Deny;
        break;
    default:
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvControlCode_Error);
        result = Event::Fail;
        break;
    }
    return result;
}

Event Protocol::recvIdle(RecvContext&)
{
    m_r1 = 0;
    return Event::Pass;
}

Event Protocol::recvPoll(RecvContext&)
{
    Event result = Event::Fail;
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::WaitForENQ);
    IODevice::Result deviceResult = m_device->poll(IODevice::kTimeoutInfinite);
    if (deviceResult == IODevice::Result::Success) {
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::LockDevice);
        m_accessLock.lock();
        result = Event::Pass;
    }
    else {
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvPoll_Error);
        result = Event::Fail;
    }
    return result;
}

Event Protocol::recvRepoll(RecvContext&)
{
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::UnlockDevice);
    m_accessLock.unlock();
    return Event::Pass;
}

Event Protocol::recvEnquiry(RecvContext&)
{
    uint8_t b = 0;
    Event result = Event::Fail;
    IODevice::Result deviceResult = m_device->read(&b, IODevice::kTimeoutImmediate);
    uint32_t bytesRead = 1;
    IODevice::Result r = deviceResult;
    while (r == IODevice::Result::Success) {
        r = m_device->read(&b, IODevice::kTimeoutImmediate);
        if (r == IODevice::Result::Timeout) {
            deviceResult = IODevice::Result::Success;
        }
        else {
            ++bytesRead;
            deviceResult = r;
        }
    }

    switch (deviceResult) {
    case IODevice::Result::Success:
        if (b == ENQ) {
            MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ReceiveEnquiryENQ, bytesRead);
            result = Event::Pass;
        }
        else if (b == NAK) {
            MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvEnquiry_Nak,
                  bytesRead);
            result = Event::Busy;
        }
        else {
            MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvEnquiry_Deny,
                  bytesRead, b);
            result = Event::Deny;
        }
        break;
    case IODevice::Result::Timeout:
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvEnquiry_Timeout);
        result = Event::Busy;
        break;
    default:
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvEnquiry_Error);
        result = Event::Fail;
        break;
    }
    return result;
}

Event Protocol::recvAcknowledgement(RecvContext&)
{
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::SendACK);
    return (std::get<IODevice::Result>(m_device->write(ACK)) == IODevice::Result::Success)
               ? Event::Pass
               : Event::Fail;
}

Event Protocol::recvLength(RecvContext& context)
{
    uint8_t b = 0;
    Event result = Event::Fail;
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::StartReceiveFrame);
    IODevice::Result deviceResult = m_device->read(&b, kTimeout4);
    switch (deviceResult) {
    case IODevice::Result::Success:
        if (b == EXT_LEN) {
            MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ReceivedExtLen);
            context.currentFrame().push_back(b);
            result = Event::Pass;
        }
        else {
            if ((b >= kMinFrameLength) && (b <= kMaxFrameLength)) {
                MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::FrameLength, b);
                context.setTransmitionType(RecvContext::TransmitionType::Regular);
                context.setDataLength(b - kFrameFooterLength);
                context.currentFrame().push_back(b);
                result = Event::Pass;
            }
            else {
                MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvLength_Deny,
                      b);
                result = Event::Deny;
            }
        }
        break;
    case IODevice::Result::Timeout:
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvLength_Timeout);
        result = Event::Deny;
        break;
    default:
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvLength_Error);
        result = Event::Fail;
        break;
    }
    return result;
}

Event Protocol::recvDataCommand(RecvContext& context)
{
    Event result = Event::Pass;
    auto& frame = context.currentFrame();
    size_t filledSize = frame.size();
    frame.resize(filledSize + kCmdLength);
    IODevice::Result deviceResult = m_device->readMulti(&frame[filledSize], kCmdLength, kTimeout4);
    uint8_t& command = frame[filledSize];
    uint8_t& subcommand = frame[filledSize + 1];

    switch (deviceResult) {
    case IODevice::Result::Success:
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ReceiveCommand, command, subcommand);
        break;
    case IODevice::Result::Timeout:
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvDataCommand_Timeout);
        result = Event::Deny;
        break;
    default:
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvDataCommand_Error);
        result = Event::Fail;
        break;
    }

    return result;
}

Event Protocol::recvDataExtLen(RecvContext& context)
{
    Event result = Event::Pass;
    if (context.transmitionType() != RecvContext::TransmitionType::Regular) {
        auto& frame = context.currentFrame();
        size_t filledSize = frame.size();
        frame.resize(filledSize + kExtLenLength);
        IODevice::Result deviceResult =
            m_device->readMulti(&frame[filledSize], kExtLenLength, kTimeout4);
        uint8_t* lengthdata = &frame[filledSize];

        switch (deviceResult) {
        case IODevice::Result::Success:
            break;
        case IODevice::Result::Timeout:
            MLOGW(common::FunctionID::cpuc_daemon_error,
                  daemon::ErrorLogID::recvDataExtLen_Timeout);
            result = Event::Deny;
            break;
        default:
            MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvDataExtLen_Error);
            result = Event::Fail;
            break;
        }
        if (result == Event::Pass) {
            uint32_t length = ((lengthdata[0] << 16) & 0x00ff0000) +
                              ((lengthdata[1] << 8) & 0x0000ff00) + ((lengthdata[2]) & 0x000000ff);

            context.setDataLength(length - kFrameFooterLength);
            MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ExtendedLength,
                  context.getDataLength());

            if ((length > kMaxExtendedLengthFrameLength) ||
                context.transmitionType() ==
                    RecvContext::TransmitionType::ExtendedLengthWithFrameDivision) {
                context.setTransmitionType(
                    RecvContext::TransmitionType::ExtendedLengthWithFrameDivision);
            }
            else {
                context.setTransmitionType(RecvContext::TransmitionType::ExtendedLength);
            }
        }
    }
    return result;
}

Event Protocol::recvDataFrameNumber(RecvContext& context)
{
    Event result = Event::Pass;
    if (context.transmitionType() ==
        RecvContext::TransmitionType::ExtendedLengthWithFrameDivision) {
        auto& frame = context.currentFrame();
        size_t filledSize = frame.size();
        frame.resize(filledSize + kFrameNumberLength + kTotalFramesLength);
        IODevice::Result deviceResult = m_device->readMulti(
            &frame[filledSize], kFrameNumberLength + kTotalFramesLength, kTimeout4);
        uint8_t* framesdata = &frame[filledSize];

        switch (deviceResult) {
        case IODevice::Result::Success:
            break;
        case IODevice::Result::Timeout:
            MLOGW(common::FunctionID::cpuc_daemon_error,
                  daemon::ErrorLogID::recvDataFrameNumber_Timeout);
            result = Event::Deny;
            break;
        default:
            MLOGW(common::FunctionID::cpuc_daemon_error,
                  daemon::ErrorLogID::recvDataFrameNumber_Error);
            result = Event::Fail;
            break;
        }
        if (result == Event::Pass) {
            uint16_t frameNumber = ((framesdata[0] << 8) & 0xff00) + (framesdata[1] & 0x00ff);
            uint16_t totalFrames = ((framesdata[2] << 8) & 0xff00) + (framesdata[3] & 0x00ff);
            context.setTotalFrames(totalFrames);
            context.setFrameNumber(frameNumber);
            MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ReceiveFrameNumber,
                  context.frameNumber(), context.totalFrames());
        }
    }
    return result;
}

Event Protocol::recvData(RecvContext& context)
{
    Event result = Event::Pass;
    uint32_t size = context.getDataLength();
    if (context.transmitionType() == RecvContext::TransmitionType::Regular) {
        size -= kCmdLength;
    }
    else if (context.transmitionType() == RecvContext::TransmitionType::ExtendedLength) {
        size -= (kCmdLength + kExtLenLength);
    }
    else {
        size -= (kCmdLength + kExtLenLength + kFrameNumberLength + kTotalFramesLength);
    }

    auto& frame = context.currentFrame();
    size_t filledSize = frame.size();
    frame.resize(filledSize + size);
    IODevice::Result deviceResult = m_device->readMulti(&frame[filledSize], size, kTimeout4);

    switch (deviceResult) {
    case IODevice::Result::Success:
        break;
    case IODevice::Result::Timeout: {
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvData_Timeout);
        result = Event::Deny;
        break;
    }
    default:
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvData_Error);
        result = Event::Fail;
        break;
    }

    if (result == Event::Pass) {
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::FrameReceived);
    }
    return result;
}

Event Protocol::recvChecksum(RecvContext& context)
{
    uint8_t calculated = common::checksum(std::next(std::begin(context.currentFrame())),
                                          std::end(context.currentFrame()));
    uint8_t b = 0;
    Event result = Event::Fail;
    IODevice::Result deviceResult = m_device->read(&b, kTimeout4);
    switch (deviceResult) {
    case IODevice::Result::Success:
        if (b == calculated) {
            MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ChecksumOK, b);
            context.currentFrame().push_back(b);
            result = Event::Pass;
        }
        else {
            MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvChecksum_Mismatch,
                  calculated, b);
            result = Event::Deny;
        }
        break;
    case IODevice::Result::Timeout:
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvChecksum_Timeout);
        result = Event::Deny;
        break;
    default:
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvChecksum_Error);
        result = Event::Fail;
        break;
    }
    return result;
}

Event Protocol::recvAcknowledgement2(RecvContext& context) { return recvAcknowledgement(context); }

Event Protocol::recvNak(RecvContext&)
{
    MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvNak);
    return (std::get<IODevice::Result>(m_device->write(NAK)) == IODevice::Result::Success)
               ? Event::Pass
               : Event::Fail;
}

Event Protocol::recvRetry(RecvContext& context)
{
    ++m_r1;
    context.currentFrame().clear();
    Event result = Event::Fail;
    if (m_r1 < kMaxNumberOfRecvAttempts) {
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::Retrying);
        result = Event::Pass;
    }
    else {
        MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvRetry_Error);
        result = Event::Fail;
    }
    return result;
}

Event Protocol::recvDone(RecvContext& context)
{
    Event result = Event::Fail;
    // check for frame division, if there are more frames - go to recvIdle
    if (context.transmitionType() == RecvContext::TransmitionType::Regular) {
        context.setResult(true);
        result = Event::Pass;
    }
    else if (context.transmitionType() == RecvContext::TransmitionType::ExtendedLength) {
        context.setResult(true);
        result = Event::Pass;
    }
    else if (context.transmitionType() ==
             RecvContext::TransmitionType::ExtendedLengthWithFrameDivision) {
        if (context.frameNumber() == context.totalFrames() - 1) {
            // done. we received all frames
            context.setResult(true);
            result = Event::Pass;
        }
        else {
            // receive next frames
            context.onFrameCompleted();
            // XXX: I do not want to introduce new Event and make state machine more complicated.
            // For now I just use Wait event to move back to idle state and receive rest of the
            // frames
            result = Event::Wait;
        }
    }
    else {
        assert(false && "Should never happen");
        result = Event::Fail;
    }

    if (result == Event::Pass) {
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ReceiveDone);
    }
    else if (result == Event::Wait) {
        MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::ReceiveProcessNextFrame);
    }
    else {
        assert(false && "Should never happen");
    }
    return result;
}

Event Protocol::recvError(RecvContext& context)
{
    MLOGW(common::FunctionID::cpuc_daemon_error, daemon::ErrorLogID::recvError);
    context.setResult(false);
    return Event::Pass;
}

Event Protocol::recvEnd(RecvContext&)
{
    MLOGV(common::FunctionID::cpuc_daemon, daemon::LogID::UnlockDevice);
    m_accessLock.unlock();
    return Event::Pass;
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
