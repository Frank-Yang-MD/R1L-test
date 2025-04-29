/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import com.mitsubishielectric.ahu.efw.cpucomservice.CpuComServiceLog;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;

/** Notify that client cant subscribe to native client due to security reasons */
public final class SubscribeTaskPermissionDeclined implements Runnable {
    private final int mPid;
    private final String mProcessName;
    private final CpuCommand mCommand;

    public SubscribeTaskPermissionDeclined(int pid, String processName, CpuCommand command) {
        mPid = pid;
        mProcessName = processName;
        mCommand = command;
    }

    @Override
    public void run() {
        MLog.i(
                MLog.CPUCOM_SERVICE_FUNCTION_ID,
                CpuComServiceLog.LogID.PermissinDeniedSubscribe,
                mCommand.cmd,
                mCommand.subCmd,
                mPid,
                mProcessName);
    }
}
