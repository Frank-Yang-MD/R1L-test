/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomstabilitytestservice;

import android.app.Service;
import android.util.Log;
import android.os.IBinder;
import android.os.Messenger;
import android.os.Message;
import android.os.Handler;
import android.os.RemoteException;
import android.os.Process;
import android.os.Bundle;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;

import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.ExecutionException;

import java.time.Instant;
import java.time.Duration;

import com.mitsubishielectric.ahu.efw.lib.common.Const;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuComManager;
import com.mitsubishielectric.ahu.efw.lib.extendedservicemanager.ExtSrvManager;

public class Test extends Service {
    private final Messenger mMessenger = new Messenger(new IncomingHandler());
    private Messenger mOutputMessenger = null;
    private ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private String mTestName;

    public static final int MESSAGE_START_TEST = 1;
    public static final int MESSAGE_STOP_TEST = 2;
    public static final int MESSAGE_TEST_RESULT = 3;

    public boolean run() {
        return false;
    }

    @Override
    public IBinder onBind(Intent intent) {
        mTestName = this.getClass().getName();
        Log.i("CLASSNAME", mTestName);
        return mMessenger.getBinder();
    }

    class IncomingHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_START_TEST:
                    mOutputMessenger = msg.replyTo;
                    connect();
                    break;
                default:
                    super.handleMessage(msg);
            }
        }
    }

    ServiceConnection serviceManagerConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            ExtSrvManager.setBinder(service);
            CpuComManager.setBinder(ExtSrvManager.getInstance().getService(Const.CPU_COM_SERVICE));
            Instant t1 = Instant.now();
            Future<Boolean> result = mExecutor.submit(() -> { return run(); });
            Message testResult = Message.obtain(null, Test.MESSAGE_TEST_RESULT);
            Bundle data = new Bundle();
            try {
                boolean b = result.get();
                data.putBoolean("result", b);
            } catch (InterruptedException e) {
                data.putBoolean("result", false);
            } catch (ExecutionException e) {
                data.putBoolean("result", false);
            }
            Instant t2 = Instant.now();
            String elapsedTime = Duration.between(t1, t2).toString();
            data.putString("elapsedTime", elapsedTime);
            data.putString("testName", mTestName);
            testResult.setData(data);
            try {
                mOutputMessenger.send(testResult);
            } catch(RemoteException e) {
            }
            unbindService(serviceManagerConnection);
        }

        public void onServiceDisconnected(ComponentName className) {
            CpuComManager.setBinder(null);
            ExtSrvManager.setBinder(null);
        }
    };

    private void connect() {
        Intent intent = new Intent();
        intent.setClassName(Const.ESM_PACKAGE, Const.ESM_SERVICE);
        //bindServiceAsUser(intent, serviceManagerConnection, BIND_AUTO_CREATE, android.os.Process.myUserHandle());
        bindService(intent, serviceManagerConnection, BIND_AUTO_CREATE);
    }
}
