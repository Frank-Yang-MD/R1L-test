/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import android.os.IBinder;
import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import java.util.Objects;
import java.util.concurrent.ExecutorService;

/**
 * Tracks the remote client lifetime through the binder to cleanup native resources when remote
 * client dies
 */
public final class ClientDeathObserver implements IBinder.DeathRecipient {

    private final ClientManager mClientManager;
    private final ExecutorService mThreadPool;
    private int mClientPid;

    public ClientDeathObserver(ClientManager clientManager, ExecutorService threadPool, int pid) {
        mClientManager =
                Objects.requireNonNull(
                        clientManager,
                        "CpuComService.ClientDeathObserver: " + "ClientManager must be not null");
        mThreadPool =
                Objects.requireNonNull(
                        threadPool,
                        "CpuComService.ClientDeathObserver: " + "ExecutorService must be not null");
        mClientPid = pid;
    }

    @Override
    public void binderDied() {
        mThreadPool.execute(new DestroyClientTask(mClientManager, mClientPid));
    }
}
