/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.RemoteException;
import com.mitsubishielectric.ahu.efw.cpucomservice.permissionchecker.PermissionChecker;
import com.mitsubishielectric.ahu.efw.lib.common.Const;
import com.mitsubishielectric.ahu.efw.lib.extendedservicemanager.ExtSrvManager;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import com.mitsubishielectric.ahu.efw.lib.vehiclepwrmgr.IVehiclePowerServiceListener;
import com.mitsubishielectric.ahu.efw.lib.vehiclepwrmgr.VehiclePowerServiceManager;
import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.concurrent.Executors;

/**
 * CpuComService.
 *
 * <p>CPU communication interface
 */
public class CpuComService extends Service {
    private CpuComServiceImpl mImpl;

    private final BroadcastReceiver mExtSrvMgnBroadcastReceiver;

    private final IVehiclePowerServiceListener mVehiclePowerServiceListener;

    private final CpuComServiceConnection mServiceConnection;

    public CpuComService() {

        // Enable permission checker
        mImpl =
                new CpuComServiceImpl(
                        Executors.newSingleThreadExecutor(),
                        new ClientManager(),
                        new PermissionChecker(this),
                        this);
        mExtSrvMgnBroadcastReceiver = new CpuComBroadcastReceiver(mImpl);
        mVehiclePowerServiceListener = new VehiclePowerServiceListener();
        mServiceConnection = new CpuComServiceConnection(this);
    }

    CpuComService(CpuComServiceImpl impl) {
        mImpl = impl;
        mExtSrvMgnBroadcastReceiver = new CpuComBroadcastReceiver(mImpl);
        mVehiclePowerServiceListener = new VehiclePowerServiceListener();
        mServiceConnection = new CpuComServiceConnection(this);
    }

    @Override
    public void onCreate() {
        MLog.initialize(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.logMessageFormats);

        super.onCreate();

        mImpl.initialize();

        IntentFilter intentFilter = new IntentFilter(Const.ESM_REBOOT_ACTION_INTENT);
        registerReceiver(
                mExtSrvMgnBroadcastReceiver, intentFilter, Const.ACCESS_MELCO_SERVICES, null);

        MLog.d(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.LogID.Created);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        mImpl.terminate();

        MLog.d(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.LogID.Destroyed);
        MLog.terminate(MLog.CPUCOM_SERVICE_FUNCTION_ID);
    }

    @Override
    public IBinder onBind(Intent intent) {
        MLog.d(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.LogID.NotBindableService);
        return null;
    }

    @Override
    protected void dump(final FileDescriptor fd, final PrintWriter pw, final String[] options) {
        pw.println("dump command execute!");
        // mImpl.dumpCommand(); //TODO (Implement native dump)
    }

    void registerService(IBinder esmBinder) {
        try {
            ExtSrvManager.setBinder(esmBinder);
            ExtSrvManager.getInstance().addService(Const.CPU_COM_SERVICE, mImpl);

            IBinder vpsBinder = ExtSrvManager.getInstance().getService(Const.VEHICLE_POWER_SERVICE);
            VehiclePowerServiceManager.setBinder(vpsBinder);
            VehiclePowerServiceManager.getInstance()
                    .subscribeFWService(
                            mVehiclePowerServiceListener, CpuComService.class.getCanonicalName());
        } catch (RemoteException rex) {
            MLog.w(
                    MLog.CPUCOM_SERVICE_FUNCTION_ID,
                    CpuComServiceLog.LogID.ExceptionRegisterService);
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        MLog.d(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLog.LogID.OnStartCommand);
        if (intent != null && intent.getExtras() != null) {
            IBinder esmBinder = intent.getExtras().getBinder(Const.ESM_SERVICE);
            registerService(esmBinder);
        } else {
            Intent esmIntent = new Intent();
            esmIntent.setClassName(Const.ESM_PACKAGE, Const.ESM_SERVICE);
            bindService(esmIntent, mServiceConnection, BIND_AUTO_CREATE);
        }
        super.onStartCommand(intent, flags, startId);
        return START_STICKY;
    }
}
