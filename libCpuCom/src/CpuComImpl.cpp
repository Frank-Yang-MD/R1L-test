/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "CpuComImpl.h"
#include "impl_CpuComImpl.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

using common::CpuCommand;

std::unique_ptr<ICpuCom> ICpuCom::create()
{
    return std::unique_ptr<ICpuCom>(new (std::nothrow) impl::CpuCom);
}

namespace impl {

class SendWorkerThreadPolicy : public common::WorkerThreadThreadPolicy {
};
class ReceiveWorkerThreadPolicy : public common::WorkerThreadThreadPolicy {
};

CpuCom::CpuCom()
{
    std::unique_ptr<common::Socket> socket(new (std::nothrow) common::Socket());
    std::unique_ptr<impl::IoDevice> ioDevice(new (std::nothrow) impl::IoDevice());
    m_impl.reset(new (std::nothrow) CpuComImpl<SendWorkerThreadPolicy, ReceiveWorkerThreadPolicy>(
        std::move(socket), std::move(ioDevice)));
}

CpuCom::~CpuCom() {}

void CpuCom::send(const CpuCommand& command, const std::vector<uint8_t>& data)
{
    return m_impl->send(command, data);
}

void CpuCom::subscribe(const CpuCommand& command, ICpuCommandListener* listener)
{
    return m_impl->subscribe(command, listener);
}

void CpuCom::subscribe(const std::list<CpuCommand>& commands, ICpuCommandListener* listener)
{
    return m_impl->subscribe(commands, listener);
}

void CpuCom::unsubscribe(const CpuCommand& command, ICpuCommandListener* listener)
{
    return m_impl->unsubscribe(command, listener);
}

void CpuCom::unsubscribe(const std::list<CpuCommand>& commands, ICpuCommandListener* listener)
{
    return m_impl->unsubscribe(commands, listener);
}

void CpuCom::setErrorCallback(ICpuCommandErrorListener* callback)
{
    return m_impl->setErrorCallback(callback);
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
