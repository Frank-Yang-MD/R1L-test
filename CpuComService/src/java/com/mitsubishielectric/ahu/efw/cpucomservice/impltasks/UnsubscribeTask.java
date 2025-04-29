/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import com.mitsubishielectric.ahu.efw.cpucomservice.Client;
import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import com.mitsubishielectric.ahu.efw.cpucomservice.CpuComServiceLog;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.Objects;

/** Unsubscribe from V-Cpu notifications */
public final class UnsubscribeTask implements Runnable {

    private final ClientManager mClientManager;
    private int mPid;
    private CpuCommand mCommand;
    private ICpuComServiceListener mListener;

    public UnsubscribeTask(
            ClientManager clientManager,
            int pid,
            CpuCommand command,
            ICpuComServiceListener listener) {
        mClientManager =
                Objects.requireNonNull(
                        clientManager,
                        "CpuComService.UnsubscribeTask: " + "ClientManager must be not null");
        this.mPid = pid;
        this.mCommand = command;
        this.mListener = listener;
    }

    @Override
    public void run() {
        Client client = mClientManager.getClient(mPid);
        if (client != null) {
            if (client.removeListener(mCommand, mListener)) {
                MLog.i(
                        MLog.CPUCOM_SERVICE_FUNCTION_ID,
                        CpuComServiceLog.LogID.Unsubscribed,
                        client.getPid(),
                        mCommand.cmd,
                        mCommand.subCmd);
            }
            if (!client.hasAliveRemoteCallbacks()) {
                mClientManager.destroyClient(client);
                MLog.i(
                        MLog.CPUCOM_SERVICE_FUNCTION_ID,
                        CpuComServiceLog.LogID.ClientDisconnected,
                        client.getPid());
            }
        }
    }
}
