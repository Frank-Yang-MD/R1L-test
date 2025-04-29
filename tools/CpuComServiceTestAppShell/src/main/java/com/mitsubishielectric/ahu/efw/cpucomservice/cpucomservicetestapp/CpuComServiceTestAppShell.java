/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.cpucomservicetestapp;

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;

import android.os.IBinder;
import android.util.Log;

import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuComManager;
import com.mitsubishielectric.ahu.efw.lib.extendedservicemanager.ExtSrvManager;

import static com.mitsubishielectric.ahu.efw.lib.common.Const.CPU_COM_SERVICE;
import static com.mitsubishielectric.ahu.efw.lib.common.Const.ESM_PACKAGE;
import static com.mitsubishielectric.ahu.efw.lib.common.Const.ESM_SERVICE;

public class CpuComServiceTestAppShell extends Service {

    public static final String APP_TAG = "CpuComServiceTestAppShell";
    private static final String INTENT_ACTION = "CpuComServiceTestApp_callApiMethod";

    private static final ComponentName mEsmComponentName = new ComponentName(ESM_PACKAGE, ESM_SERVICE);

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(APP_TAG, "onStartCommand");

        bindToEsm();

        return Service.START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private final ServiceConnection mEsmConnectionCallback = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.i(APP_TAG, "ESM Service connected");
            ExtSrvManager.setBinder(service);

            IBinder cpuComServiceBinder = ExtSrvManager.getInstance().getService(CPU_COM_SERVICE);
            if (cpuComServiceBinder == null) {
                Log.i(APP_TAG, "Can't set CpuComServiceBinder. Binder is null");
            } else {
                Log.i(APP_TAG, "Successfully set binder to CpuComManager");
                CpuComManager.setBinder(cpuComServiceBinder);
                registerReceiver(new ApiCallIntentReceiver(), new IntentFilter(INTENT_ACTION));
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.i(APP_TAG, "ESM Service disconnected");
        }
    };

    private void bindToEsm() {
        Intent esmBindIntent = new Intent();
        esmBindIntent.setComponent(mEsmComponentName);

        if (bindService(esmBindIntent, mEsmConnectionCallback, BIND_AUTO_CREATE)) {
            Log.i(APP_TAG, "bindService(ESM) returned - OK");
        } else {
            Log.i(APP_TAG, "bindService(ESM) returned - FAILED");
        }
    }
}
