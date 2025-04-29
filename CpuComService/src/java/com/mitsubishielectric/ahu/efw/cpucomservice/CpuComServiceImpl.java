/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import android.content.Context;
import android.os.Binder;
import com.mitsubishielectric.ahu.efw.cpucomservice.impltasks.ResetErrorCallbackTask;
import com.mitsubishielectric.ahu.efw.cpucomservice.impltasks.SendCommandTask;
import com.mitsubishielectric.ahu.efw.cpucomservice.impltasks.SendCommandTaskPermissionDeclined;
import com.mitsubishielectric.ahu.efw.cpucomservice.impltasks.SetErrorCallbackTask;
import com.mitsubishielectric.ahu.efw.cpucomservice.impltasks.SubscribeTask;
import com.mitsubishielectric.ahu.efw.cpucomservice.impltasks.SubscribeTaskPermissionDeclined;
import com.mitsubishielectric.ahu.efw.cpucomservice.impltasks.UnsubscribeTask;
import com.mitsubishielectric.ahu.efw.cpucomservice.permissionchecker.IPermissionChecker;
import com.mitsubishielectric.ahu.efw.cpucomservice.permissionchecker.PermissionCheckerDefault;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComService;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/** Implementation of CpuCOomService API */
class CpuComServiceImpl extends ICpuComService.Stub {

    private final ExecutorService mThreadPool;
    private final ClientManager mClientManager;
    private final IPermissionChecker mPermissionChecker;
    private final Context mContext;

    CpuComServiceImpl() {
        mThreadPool = Executors.newSingleThreadExecutor();
        mClientManager = new ClientManager();
        mPermissionChecker = new PermissionCheckerDefault();
        mContext = null;
    }

    CpuComServiceImpl(
            ExecutorService threadPool,
            ClientManager clientManager,
            IPermissionChecker permissionChecker,
            Context context) {
        mThreadPool = threadPool;
        mClientManager = clientManager;
        mPermissionChecker = permissionChecker;
        mContext = context;
    }

    private String getCallingProcessName() {
        if (mContext != null) {
            return mContext.getPackageManager().getNameForUid(Binder.getCallingUid());
        } else {
            return "";
        }
    }

    public boolean initialize() {
        return true;
    }

    public void terminate() {
        mThreadPool.shutdown();
    }

    public void sendCmd(CpuCommand command) {
        int pid = Binder.getCallingPid();
        String processName = getCallingProcessName();

        if (mPermissionChecker.isAccessGranted(command)) {
            mThreadPool.execute(new SendCommandTask(mClientManager, pid, command));
        } else {
            mThreadPool.execute(new SendCommandTaskPermissionDeclined(pid, processName, command));
        }
    }

    public void subscribeCB(CpuCommand command, ICpuComServiceListener listener) {
        int pid = Binder.getCallingPid();
        String processName = getCallingProcessName();

        if (mPermissionChecker.isAccessGranted(command)) {
            mThreadPool.execute(
                    new SubscribeTask(mClientManager, mThreadPool, pid, command, listener));
        } else {
            mThreadPool.execute(new SubscribeTaskPermissionDeclined(pid, processName, command));
        }
    }

    public void unsubscribeCB(CpuCommand command, ICpuComServiceListener listener) {
        int pid = Binder.getCallingPid();
        mThreadPool.execute(new UnsubscribeTask(mClientManager, pid, command, listener));
    }

    public void subscribeErrorCB(ICpuComServiceErrorListener errorCallback) {
        int pid = Binder.getCallingPid();
        mThreadPool.execute(
                new SetErrorCallbackTask(mClientManager, mThreadPool, pid, errorCallback));
    }

    public void unsubscribeErrorCB(ICpuComServiceErrorListener errorCallback) {
        int pid = Binder.getCallingPid();
        mThreadPool.execute(new ResetErrorCallbackTask(mClientManager, pid));
    }

    public void subscribeListCB(List<CpuCommand> commands, ICpuComServiceListener listener) {
        for (CpuCommand command : commands) {
            subscribeCB(command, listener);
        }
    }

    public void unsubscribeListCB(List<CpuCommand> commands, ICpuComServiceListener listener) {
        MLog.i(
                MLog.CPUCOM_SERVICE_FUNCTION_ID,
                CpuComServiceLog.LogID.ListUnsubscribing,
                Binder.getCallingPid(),
                commands.size());
        for (CpuCommand command : commands) {
            unsubscribeCB(command, listener);
        }
    }
}
