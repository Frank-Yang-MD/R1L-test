/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import com.mitsubishielectric.ahu.efw.cpucomservice.Client;
import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import com.mitsubishielectric.ahu.efw.cpucomservice.CpuComServiceLog;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.Objects;
import java.util.concurrent.ExecutorService;

/** Set error callback listener to native client */
public final class SetErrorCallbackTask implements Runnable {

    private final ClientManager mClientManager;
    private final ExecutorService mThreadPool;
    private int mPid;
    private ICpuComServiceErrorListener mErrorCallback;

    public SetErrorCallbackTask(
            ClientManager clientManager,
            ExecutorService threadPool,
            int pid,
            ICpuComServiceErrorListener errorCallback) {
        mClientManager =
                Objects.requireNonNull(
                        clientManager,
                        "CpuComService.SetErrorCallbackTask: " + "ClientManager must be not null");
        mThreadPool =
                Objects.requireNonNull(
                        threadPool,
                        "CpuComService.SetErrorCallbackTask: "
                                + "ExecutorService must be not null");
        this.mPid = pid;
        mErrorCallback =
                Objects.requireNonNull(
                        errorCallback,
                        "puComService.SetErrorCallbackTask: " + "ErrorCallback must be not null");
    }

    @Override
    public void run() {
        Client client = mClientManager.getClient(mPid);
        if (client == null) {
            client =
                    mClientManager.createClient(
                            mPid,
                            mErrorCallback.asBinder(),
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
            client.setErrorCallback(mErrorCallback);
            MLog.i(
                    MLog.CPUCOM_SERVICE_FUNCTION_ID,
                    CpuComServiceLog.LogID.RegisteredErrorCallBack,
                    client.getPid());
        }
    }
}
