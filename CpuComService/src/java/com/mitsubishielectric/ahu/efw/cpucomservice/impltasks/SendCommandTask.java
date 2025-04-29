/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import com.mitsubishielectric.ahu.efw.cpucomservice.Client;
import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import com.mitsubishielectric.ahu.efw.cpucomservice.CpuComServiceLog;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.Objects;

/** Send V-Cpu command to native client */
public final class SendCommandTask implements Runnable {

    private final ClientManager mClientManager;
    private final int mPid;
    private final CpuCommand mCommand;

    public SendCommandTask(ClientManager clientManager, int pid, CpuCommand command) {
        mClientManager =
                Objects.requireNonNull(
                        clientManager,
                        "CpuComService.SendCommandTask: " + "Client manager must be not null");

        mPid = pid;
        mCommand =
                Objects.requireNonNull(
                        command, "CpuComService.SendCommandTask: " + "CpuCommand must be not null");
    }

    @Override
    public void run() {
        if (mCommand.isValid()) {
            Client client = mClientManager.getClient(mPid);
            if (client == null) {
                client = mClientManager.getAnonymousClient();
            }

            client.send(mCommand);
            MLog.i(
                    MLog.CPUCOM_SERVICE_FUNCTION_ID,
                    CpuComServiceLog.LogID.Send,
                    client.getPid(),
                    mCommand.cmd,
                    mCommand.subCmd);
        }
    }
}
