/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */
// LCOV_EXCL_START
// This is excluded from a unit test coverage report because this class
// only delegate all calls to Messenger class

#include "CpuComMessenger.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

CpuComMessenger::CpuComMessenger(std::string daemonSocketName,
                                 std::unique_ptr<common::Socket> socket,
                                 std::unique_ptr<common::IPausableExecutor> incomingExecutor,
                                 std::unique_ptr<common::IExecutor> outcomingExecutor,
                                 bool reconnect)
    : mMessenger{std::make_unique<IMessenger::Messenger>(daemonSocketName,
                                                         std::move(socket),
                                                         std::move(incomingExecutor),
                                                         std::move(outcomingExecutor),
                                                         reconnect)}
{
}

bool CpuComMessenger::initialize(OnConnectionClosedHandler onConnectionClosed,
                                 OnConnectionResumedHandler onConnectionResumed)
{
    return mMessenger->initialize(onConnectionClosed, onConnectionResumed);
}

bool CpuComMessenger::connect() { return mMessenger->connect(); }

void CpuComMessenger::disconnect() { mMessenger->disconnect(); }

void CpuComMessenger::setSendCommandResultMessageHandler(OnSendCommandResultHandler handler,
                                                         v2::CpuCom* cpuCom)
{
    mMessenger->setMessageHandler(CpuComId::SendCommandResult, handler, cpuCom);
}

void CpuComMessenger::setNotificationMessageHandler(OnNotificationHandler handler,
                                                    v2::CpuCom* cpuCom)
{
    mMessenger->setMessageHandler(CpuComId::Notification, handler, cpuCom);
}

void CpuComMessenger::setRequestResponseMessageHandler(OnRequestResponseHandler handler,
                                                       v2::CpuCom* cpuCom)
{
    mMessenger->setMessageHandler(CpuComId::RequestResponse, handler, cpuCom);
}

void CpuComMessenger::setDeliveryStatusMessageHandler(OnDeliveryStatusHandler handler,
                                                      v2::CpuCom* cpuCom)
{
    mMessenger->setMessageHandler(CpuComId::DeliveryStatus, handler, cpuCom);
}

void CpuComMessenger::sendCancelRequestMessage(common::UUID uuid)
{
    mMessenger->sendMessage(CpuComId::CancelRequest, std::move(uuid));
}

void CpuComMessenger::sendSubscribeMessage(common::CpuCommand command)
{
    mMessenger->sendMessage(CpuComId::Subscribe, std::move(command));
}

void CpuComMessenger::sendUnsubscribeMessage(common::CpuCommand command)
{
    mMessenger->sendMessage(CpuComId::Unsubscribe, std::move(command));
}

void CpuComMessenger::sendSendCommandMessage(common::CpuCommand command, std::vector<uint8_t> data)
{
    mMessenger->sendMessage(CpuComId::SendCommand, std::move(command), std::move(data));
}

void CpuComMessenger::sendSendCommandWithDeliveryStatusMessage(common::UUID uuid,
                                                               common::CpuCommand command,
                                                               std::vector<uint8_t> data)
{
    mMessenger->sendMessage(CpuComId::SendCommandWithDeliveryStatus, std::move(uuid),
                            std::move(command), std::move(data));
}

void CpuComMessenger::sendRequestMessage(common::UUID uuid,
                                         common::CpuCommand requestCommand,
                                         std::vector<uint8_t> data,
                                         common::CpuCommand responseCommand)
{
    mMessenger->sendMessage(CpuComId::Request, std::move(uuid), std::move(requestCommand),
                            std::move(data), std::move(responseCommand));
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

// LCOV_EXCL_STOP
