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
import java.util.concurrent.ExecutorService;

/** Subscribe to receive notification from V-Cpu */
public final class SubscribeTask implements Runnable {
    private final ClientManager mClientManager;
    private final ExecutorService mThreadPool;
    private int mPid;
    private CpuCommand mCommand;
    private ICpuComServiceListener mListener;

    public SubscribeTask(
            ClientManager clientManager,
            ExecutorService threadPool,
            int pid,
            CpuCommand command,
            ICpuComServiceListener listener) {
        mClientManager =
                Objects.requireNonNull(
                        clientManager,
                        "CpuComService.SubscribeTask: " + "ClientManager must be not null");
        mThreadPool =
                Objects.requireNonNull(
                        threadPool,
                        "CpuComService.SubscribeTask: " + "ExecutorService must be not null");
        this.mPid = pid;
        this.mCommand = command;
        this.mListener = listener;
    }

    @Override
    public void run() {
        if (mCommand.isValid()) {
            Client client = mClientManager.getClient(mPid);
            if (client == null) {
                client =
                        mClientManager.createClient(
                                mPid,
                                mListener.asBinder(),
                                new ClientDeathObserver(mClientManager, mThreadPool, mPid));
                if (client != null) {
                    MLog.i(
                            MLog.CPUCOM_SERVICE_FUNCTION_ID,
                            CpuComServiceLog.LogID.ClientConnected,
                            client.getPid());
                } else {
                    MLog.i(
                            MLog.CPUCOM_SERVICE_FUNCTION_ID,
                            CpuComServiceLog.LogID.CanNotCreateClient,
                            mPid);
                }
            }
            if (client != null) {
                client.addListener(mCommand, mListener);
                MLog.i(
                        MLog.CPUCOM_SERVICE_FUNCTION_ID,
                        CpuComServiceLog.LogID.Subscribed,
                        client.getPid(),
                        mCommand.cmd,
                        mCommand.subCmd);
            }
        } else {
            MLog.w(
                    MLog.CPUCOM_SERVICE_FUNCTION_ID,
                    CpuComServiceLog.LogID.InvalidCommandSubscribe,
                    mPid,
                    mCommand.cmd,
                    mCommand.subCmd);
        }
    }
}
