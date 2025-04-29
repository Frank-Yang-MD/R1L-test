/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import android.os.RemoteCallbackList;
import android.os.RemoteException;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.Objects;

public class NativeErrorCallbackWrapper {
    private final RemoteCallbackList<ICpuComServiceErrorListener> mCallback;
    private int mPid;

    public NativeErrorCallbackWrapper(
            int clientPid, RemoteCallbackList<ICpuComServiceErrorListener> listeners) {
        mPid = clientPid;

        mCallback =
                Objects.requireNonNull(
                        listeners,
                        "CpuComService.NativeErrorCallbackWrapper: "
                                + "Listeners must be not null");
    }

    public void destroy() {
        mCallback.kill();
    }

    // Called from native thread. JNI part guaranties that onCommand and onError callback will
    // always be called from the same thread.
    public void onError(int cmd, int subcmd, int errorCode) {
        CpuCommand command = new CpuCommand(cmd, subcmd, null);
        int count = mCallback.beginBroadcast();
        for (int i = 0; i < count; ++i) {
            try {
                mCallback.getBroadcastItem(i).onError(errorCode, command);
                MLog.d(
                        MLog.CPUCOM_SERVICE_FUNCTION_ID,
                        CpuComServiceLog.LogID.OnError,
                        command.cmd,
                        command.subCmd,
                        mPid);
            } catch (final RemoteException ex) {
                MLog.d(
                        MLog.CPUCOM_SERVICE_FUNCTION_ID,
                        CpuComServiceLog.LogID.ExceptionWhileOnErrorBroadcast,
                        command.cmd,
                        command.subCmd,
                        mPid);
            }
        }
        mCallback.finishBroadcast();
    }
}
