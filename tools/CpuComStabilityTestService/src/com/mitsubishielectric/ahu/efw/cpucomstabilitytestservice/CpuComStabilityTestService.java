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

import java.util.*;

public class CpuComStabilityTestService extends Service {
    private static final String TAG = "CpuComStabilityTest";

    // Tests that are in this list will be run by stability test service.
    private List<Class> mTests = Arrays.asList(
        // SendReceiveTest.class
        SubscribeSendTest0.class,
        SubscribeSendTest1.class,
        SubscribeSendTest2.class
    );

    private Map<String, ServiceConnection> mConnections = new HashMap<String, ServiceConnection>();

    final Messenger mInputMessenger = new Messenger(new IncomingHandler());

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "STARTED");
        for (Class test : mTests) {
            Log.i(TAG, test.getName());
            mConnections.put(test.getName(), null);
        }
        this.startTests();
        return START_REDELIVER_INTENT;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    class IncomingHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case SendReceiveTest.MESSAGE_TEST_RESULT:
                    String testName = msg.getData().getString("testName");
                    Boolean result = msg.getData().getBoolean("result");
                    String elapsedTime = msg.getData().getString("elapsedTime");
                    if (result == true) {
                        Log.i(TAG, testName + ": result = SUCCESS. elapsed time = " + elapsedTime);
                    } else {
                        Log.i(TAG, testName + ": result = FAILED.  elapsed time = " + elapsedTime);
                    }
                    unbindService(mConnections.get(testName));
                    mConnections.remove(testName);
                    if (mConnections.isEmpty()) {
                        Log.i(TAG, "FINISHED");
                        stopSelf();
                    }
                    break;
                default:
                    super.handleMessage(msg);
            }
        }
    }

    private void startTests() {
        for (Class test : mTests) {
            mConnections.put(test.getName(), new ServiceConnection() {
                public void onServiceConnected(ComponentName className, IBinder service) {
                    Messenger outputMessenger = new Messenger(service);
                    try {
                        Message msg = Message.obtain(null, SendReceiveTest.MESSAGE_START_TEST);
                        msg.replyTo = mInputMessenger;
                        outputMessenger.send(msg);
                        Log.i(TAG, "Started: " + className.toShortString());
                    } catch (RemoteException e) {
                    }
                }

                public void onServiceDisconnected(ComponentName className) {
                }
            });

            //bindServiceAsUser(new Intent(this, test), mConnections.get(test.getName()), Context.BIND_AUTO_CREATE, android.os.Process.myUserHandle());
            bindService(new Intent(this, test), mConnections.get(test.getName()), Context.BIND_AUTO_CREATE);
        }
    }
}