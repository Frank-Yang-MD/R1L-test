/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import android.os.RemoteCallbackList;
import android.os.RemoteException;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.Objects;

public class NativeCallbackWrapper {
    private final RemoteCallbackList<ICpuComServiceListener> mCallback;
    private int mPid;

    public NativeCallbackWrapper(
            int clientPid, RemoteCallbackList<ICpuComServiceListener> listeners) {
        mPid = clientPid;
        mCallback =
                Objects.requireNonNull(
                        listeners,
                        "CpuComService.NativeCallbackWrapper: " + "Listeners must be not null");
    }

    public void destroy() {
        mCallback.kill();
    }

    public void put(ICpuComServiceListener listener) {
        mCallback.register(listener);
    }

    public void remove(ICpuComServiceListener listener) {
        mCallback.unregister(listener);
    }

    public boolean isEmpty() {
        return mCallback.getRegisteredCallbackCount() == 0;
    }

    // Called from native thread. JNI part guaranties that onCommand and onError callback will
    // always be called from the same thread.
    public void onCommand(int cmd, int subcmd, final byte[] data) {
        CpuCommand command = new CpuCommand(cmd, subcmd, data);
        int count = mCallback.beginBroadcast();
        for (int i = 0; i < count; ++i) {
            try {
                mCallback.getBroadcastItem(i).onReceiveCmd(command);
                MLog.d(
                        MLog.CPUCOM_SERVICE_FUNCTION_ID,
                        CpuComServiceLog.LogID.OnCommand,
                        command.cmd,
                        command.subCmd,
                        mPid);
            } catch (final RemoteException ex) {
                MLog.d(
                        MLog.CPUCOM_SERVICE_FUNCTION_ID,
                        CpuComServiceLog.LogID.ExceptionWhileOnCommandBroadcast,
                        command.cmd,
                        command.subCmd,
                        mPid);
            }
        }
        mCallback.finishBroadcast();
    }
}
