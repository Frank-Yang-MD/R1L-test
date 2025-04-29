/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import com.mitsubishielectric.ahu.efw.lib.common.Const;
import com.mitsubishielectric.ahu.efw.lib.extendedservicemanager.ExtSrvManager;
import java.util.Objects;

/** This class is responsible for register service in the ESM */
public final class CpuComBroadcastReceiver extends BroadcastReceiver {
    private CpuComServiceImpl mCpuComServiceImpl;

    public CpuComBroadcastReceiver(CpuComServiceImpl cpuComServiceImpl) {
        mCpuComServiceImpl =
                Objects.requireNonNull(
                        cpuComServiceImpl,
                        "CpuComService.CpuComBroadcastReceiver: CpuComServiceImpl must be not null");
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        ExtSrvManager.setBinder(intent.getExtras().getBinder(Const.ESM_SERVICE));
        ExtSrvManager.getInstance().addService(Const.CPU_COM_SERVICE, mCpuComServiceImpl);
    }
}
