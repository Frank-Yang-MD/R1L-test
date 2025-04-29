/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import com.mitsubishielectric.ahu.efw.cpucomservice.CpuComServiceLog;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;

/** Notify that V-Cpu command can't be send to native client due to security reasons */
public final class SendCommandTaskPermissionDeclined implements Runnable {
    private final int mPid;
    private final String mProcessName;
    private final CpuCommand mCommand;

    public SendCommandTaskPermissionDeclined(int pid, String processName, CpuCommand command) {
        mPid = pid;
        mCommand = command;
        mProcessName = processName;
    }

    @Override
    public void run() {
        MLog.i(
                MLog.CPUCOM_SERVICE_FUNCTION_ID,
                CpuComServiceLog.LogID.PermissinDeniedSend,
                mCommand.cmd,
                mCommand.subCmd,
                mPid,
                mProcessName);
    }
}
