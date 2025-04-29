/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import com.mitsubishielectric.ahu.efw.cpucomservice.Client;
import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import com.mitsubishielectric.ahu.efw.cpucomservice.CpuComServiceLog;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.Objects;

/** Reset error callback listener to native client */
public final class ResetErrorCallbackTask implements Runnable {

    private final ClientManager mClientManager;
    private int mPid;

    public ResetErrorCallbackTask(ClientManager clientManager, int pid) {
        mClientManager =
                Objects.requireNonNull(
                        clientManager,
                        "CpuComService.ResetErrorCallbackTask: "
                                + "ClientManager must be not null");
        this.mPid = pid;
    }

    @Override
    public void run() {
        Client client = mClientManager.getClient(mPid);
        if (client != null) {
            client.setErrorCallback(null);
            MLog.i(
                    MLog.CPUCOM_SERVICE_FUNCTION_ID,
                    CpuComServiceLog.LogID.UnregisteredErrorCallBack,
                    client.getPid());

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
