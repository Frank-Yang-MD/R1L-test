/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomstabilitytestservice;

import android.os.IBinder;

import android.util.Log;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.ExecutionException;

import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuComManager;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;

public class SendReceiveTest extends Test {
    private final int mSendsCount = 10;

    private final class Listener extends ICpuComServiceListener.Stub {
        public int mReceivesCount = 0;
        @Override
        public void onReceiveCmd(CpuCommand command) {
            ++mReceivesCount;
        }
    }
    private final Listener mListener0 = new Listener();
    private final Listener mListener1 = new Listener();

    public boolean run() {
        CpuCommand echoRequest = new CpuCommand(0xfd, 0x01, new byte[128]);
        CpuCommand echoResponse = new CpuCommand(0xfd, 0x81, null);
        CpuComManager.getInstance().subscribeCB(echoResponse, mListener0);
        CpuComManager.getInstance().subscribeCB(echoResponse, mListener1);
        for (int i = 0; i < mSendsCount; ++i) {
            CpuComManager.getInstance().sendCmd(echoRequest);
        }

        // Timeout was choosen empirically, not the best approach, but good enough for this test.
        try {
            Thread.sleep(1000 * mSendsCount);
        } catch (InterruptedException e) {
        }
        CpuComManager.getInstance().unsubscribeCB(echoResponse, mListener1);
        CpuComManager.getInstance().unsubscribeCB(echoResponse, mListener0);
        return (mListener0.mReceivesCount == mSendsCount) && (mListener1.mReceivesCount == mSendsCount);
    }
}
