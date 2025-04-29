/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import android.os.IBinder;
import android.os.RemoteCallbackList;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;
import java.util.HashMap;
import java.util.Map;

/** Represents a CpuComService's remote client. */
public class Client {
    // Process ID, to associate this client with a native ICpuCom interface and with a remote
    // process in sendCmd call.
    private final int mPid;
    // IBinder interface used to track remote client's lifetime and cleanup allocated native
    // resources
    // when remote client dies or disconnects
    private final IBinder mBinder;
    private final Map<CpuCommand, NativeCallbackWrapper> mCallbacks =
            new HashMap<CpuCommand, NativeCallbackWrapper>();
    private NativeErrorCallbackWrapper mErrorCallback = null;

    private native void create(int pid);

    private native void destroy(int pid);

    private native void subscribe(int pid, int cmd, int subCmd, Object object);

    private native void unsubscribe(int pid, int cmd, int subCmd, Object object);

    private native void setErrorCallback(int pid, Object object);

    private native void send(int pid, int cmd, int subCmd, byte[] data);

    public Client(int pid, IBinder binder) {
        mPid = pid;
        mBinder = binder;
        create(pid);
    }

    public int getPid() {
        return mPid;
    }

    public IBinder getBinder() {
        return mBinder;
    }

    public void destroy() {
        destroy(mPid);
    }

    public void addListener(CpuCommand command, ICpuComServiceListener listener) {
        NativeCallbackWrapper wrapper = null;
        synchronized (mCallbacks) {
            wrapper =
                    mCallbacks.computeIfAbsent(
                            command,
                            k -> new NativeCallbackWrapper(mPid, new RemoteCallbackList<>()));
        }
        boolean isSubscribed = !wrapper.isEmpty();
        wrapper.put(listener);
        if (!isSubscribed) {
            subscribe(mPid, command.cmd, command.subCmd, wrapper);
        }
    }

    public boolean removeListener(CpuCommand command, ICpuComServiceListener listener) {
        NativeCallbackWrapper wrapper = null;
        synchronized (mCallbacks) {
            wrapper = mCallbacks.get(command);
        }
        if (wrapper != null) {
            wrapper.remove(listener);
            if (wrapper.isEmpty()) {
                unsubscribe(mPid, command.cmd, command.subCmd, wrapper);
                synchronized (mCallbacks) {
                    mCallbacks.remove(command);
                }
            }
            return true;
        }
        return false;
    }

    public void setErrorCallback(ICpuComServiceErrorListener listener) {
        NativeErrorCallbackWrapper newErrorCallback = null;
        if (listener != null) {
            final RemoteCallbackList<ICpuComServiceErrorListener> listenersList =
                    new RemoteCallbackList<>();
            listenersList.register(listener);

            newErrorCallback = new NativeErrorCallbackWrapper(mPid, listenersList);
        }
        setErrorCallback(mPid, newErrorCallback);
        if (mErrorCallback != null) {
            mErrorCallback.destroy();
        }
        mErrorCallback = newErrorCallback;
    }

    public void send(CpuCommand command) {
        send(mPid, command.cmd, command.subCmd, command.data);
    }

    public boolean hasAliveRemoteCallbacks() {
        return !mCallbacks.isEmpty() || (mErrorCallback != null);
    }
}
