/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.lib.cpucomservice;

import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;

/**
 * CPU Communication Service I/F
 */
interface ICpuComService {
    /**
     * Send command to vcpu
     */
    void sendCmd(in CpuCommand command);

    /**
     * Subscribe on VCPU command
     */
    void subscribeCB(in CpuCommand command, ICpuComServiceListener listener);

    /**
     * Subscribe on list of VCPU commands
     */
    void subscribeListCB(in List<CpuCommand> commands, ICpuComServiceListener listener);

    /**
     * Subscribe on send and subscribe command errors
     */
    void subscribeErrorCB(in ICpuComServiceErrorListener errorListener);

    /**
     * Unsubscribe from VCPU command
     */
    void unsubscribeCB(in CpuCommand command, ICpuComServiceListener listener);

    /**
     * Unsubscribe from send and subscribe command errors
     */
    void unsubscribeErrorCB(in ICpuComServiceErrorListener listener);

    /**
     * Unsubscribe from list of VCPU commands
     */
    void unsubscribeListCB(in List<CpuCommand> commands, ICpuComServiceListener listener);
}
