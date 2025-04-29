/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import com.mitsubishielectric.ahu.efw.cpucomservice.Client;
import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import java.util.Objects;

/** Cleanup native resources when remote client dies */
public final class DestroyClientTask implements Runnable {

    private final ClientManager mClientManager;
    private int mPid;

    public DestroyClientTask(ClientManager clientManager, int pid) {
        mClientManager =
                Objects.requireNonNull(
                        clientManager,
                        "CpuComService.DestroyClientTask: " + "ClientManager must be not null");
        this.mPid = pid;
    }

    @Override
    public void run() {
        Client client = mClientManager.getClient(mPid);
        mClientManager.destroyClient(client);
    }
}
