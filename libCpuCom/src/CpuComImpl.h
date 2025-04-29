/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMIMPL_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMIMPL_H_

#include "CpuCom.h"

#include <memory>

#include "Socket.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class SendWorkerThreadPolicy;
class ReceiveWorkerThreadPolicy;
template <typename SendThreadPolicy, typename ReceiveThreadPolicy>
class CpuComImpl;
class CpuCom : public ICpuCom {
public:
    explicit CpuCom();
    virtual ~CpuCom();

public:
    virtual void send(const common::CpuCommand& command, const std::vector<uint8_t>& data) override;
    virtual void subscribe(const common::CpuCommand& command,
                           ICpuCommandListener* listener) override;
    virtual void subscribe(const std::list<common::CpuCommand>& commands,
                           ICpuCommandListener* listener) override;
    virtual void unsubscribe(const common::CpuCommand& command,
                             ICpuCommandListener* listener) override;
    virtual void unsubscribe(const std::list<common::CpuCommand>& commands,
                             ICpuCommandListener* listener) override;
    virtual void setErrorCallback(ICpuCommandErrorListener* callback) override;

private:
    std::unique_ptr<CpuComImpl<SendWorkerThreadPolicy, ReceiveWorkerThreadPolicy>> m_impl;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMIMPL_H_
