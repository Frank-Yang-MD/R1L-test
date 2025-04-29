/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import android.os.RemoteException;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import com.mitsubishielectric.ahu.efw.lib.vehiclepwrmgr.IVehiclePowerServiceListener;
import com.mitsubishielectric.ahu.efw.lib.vehiclepwrmgr.VehiclePowerServiceManager;

/** This class is responsible for subscribing to events from the VPS */
public final class VehiclePowerServiceListener extends IVehiclePowerServiceListener.Stub {

    @Override
    public void onAppStart() {
        MLog.d(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.LogID.OnAppStart);
        try {
            VehiclePowerServiceManager.getInstance()
                    .startComplete(CpuComService.class.getCanonicalName());
        } catch (RemoteException ex) {
            MLog.w(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.LogID.ExceptionStartComplete);
        }
    }

    @Override
    public void onAppRestart() {
        MLog.d(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.LogID.OnAppRestart);
        try {
            VehiclePowerServiceManager.getInstance()
                    .restartCompleteEfw(CpuComService.class.getCanonicalName());
        } catch (RemoteException ex) {
            MLog.w(
                    MLog.CPUCOM_SERVICE_FUNCTION_ID,
                    CpuComServiceLog.LogID.ExceptionRestartComplete);
        }
    }

    @Override
    public void onAppStop() {
        MLog.d(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.LogID.OnAppStop);
        try {
            VehiclePowerServiceManager.getInstance()
                    .stopCompleteEfw(CpuComService.class.getCanonicalName());
        } catch (RemoteException ex) {
            MLog.w(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.LogID.ExceptionStopComplete);
        }
    }

    @Override
    public void onAppResume() {
        MLog.d(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.LogID.OnAppResume);
        try {
            VehiclePowerServiceManager.getInstance()
                    .resumeCompleteEfw(CpuComService.class.getCanonicalName());
        } catch (RemoteException ex) {
            MLog.w(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.LogID.ExceptionResumeComplete);
        }
    }
}
