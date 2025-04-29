/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */
// LCOV_EXCL_START
// This is excluded from a unit test coverage report because this class
// only delegate all calls to MessageServer class

#include "CpuComMessageServer.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using common::IExecutor;
using common::SingleThreadExecutor;

CpuComMessageServer::CpuComMessageServer(
    std::string socketName,
    const std::unordered_set<std::string>& securityContextExceptions,
    std::unique_ptr<common::IExecutor> incomingExecutor,
    std::unique_ptr<common::IExecutor> outcomingExecutor,
    OnStartedHandler onStartedHandler,
    OnStoppedHandler onStoppedHandler)
    : mMessageServer{std::make_unique<IMessageServer::MessageServer>(socketName,
                                                                     securityContextExceptions,
                                                                     std::move(incomingExecutor),
                                                                     std::move(outcomingExecutor),
                                                                     onStartedHandler,
                                                                     onStoppedHandler)}
{
}

bool CpuComMessageServer::initialize(OnNewConnectionHandler onNewConnectionHandler,
                                     OnConnectionClosedHandler onConnectionClosedHandler)
{
    return mMessageServer->initialize(onNewConnectionHandler, onConnectionClosedHandler);
}

bool CpuComMessageServer::start() { return mMessageServer->start(); }

void CpuComMessageServer::stop() { mMessageServer->stop(); }

void CpuComMessageServer::setSendCommandMessageHandler(OnSendCommandHandler handler,
                                                       CpuComDaemon* daemon)
{
    mMessageServer->setMessageHandler(CpuComId::SendCommand, handler, daemon);
}

void CpuComMessageServer::setSubscribeMessageHandler(OnSubscribeHandler handler,
                                                     CpuComDaemon* daemon)
{
    mMessageServer->setMessageHandler(CpuComId::Subscribe, handler, daemon);
}

void CpuComMessageServer::setUnsubscribeMessageHandler(OnUnsubscribeHandler handler,
                                                       CpuComDaemon* daemon)
{
    mMessageServer->setMessageHandler(CpuComId::Unsubscribe, handler, daemon);
}

void CpuComMessageServer::setRequestMessageHandler(OnRequestHandler handler, CpuComDaemon* daemon)
{
    mMessageServer->setMessageHandler(CpuComId::Request, handler, daemon);
}

void CpuComMessageServer::setCancelRequestMessageHandler(OnCancelRequestHandler handler,
                                                         CpuComDaemon* daemon)
{
    mMessageServer->setMessageHandler(CpuComId::CancelRequest, handler, daemon);
}

void CpuComMessageServer::setSendCommandWithDeliveryStatusMessageHandler(
    OnSendCommandWithDeliveryStatusHandler handler,
    CpuComDaemon* daemon)
{
    mMessageServer->setMessageHandler(CpuComId::SendCommandWithDeliveryStatus, handler, daemon);
}

void CpuComMessageServer::sendNotificationMessage(SessionID sessionId,
                                                  common::CpuCommand command,
                                                  std::vector<uint8_t>& data)
{
    mMessageServer->sendMessage(sessionId, CpuComId::Notification, command, data);
}

void CpuComMessageServer::sendRequestResponseMessage(SessionID sessionId,
                                                     common::UUID uuid,
                                                     std::vector<uint8_t>& data)
{
    mMessageServer->sendMessage(sessionId, CpuComId::RequestResponse, uuid, data);
}

void CpuComMessageServer::sendSendCommandResultMessage(SessionID sessionId,
                                                       common::CpuCommand command,
                                                       common::Error error)
{
    mMessageServer->sendMessage(sessionId, CpuComId::SendCommandResult, command, error);
}

void CpuComMessageServer::sendDeliveryStatusMessage(SessionID sessionId,
                                                    common::UUID uuid,
                                                    bool result)
{
    mMessageServer->sendMessage(sessionId, CpuComId::DeliveryStatus, uuid, result);
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

// LCOV_EXCL_STOP