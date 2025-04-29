/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomstabilitytestservice;

import android.util.Log;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.ExecutionException;

import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuComManager;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;

public class SubscribeSendTest2 extends Test {
    private final int mSendsCount = 2000;
    private CountDownLatch mLatch = null;

    private final ICpuComServiceListener mListener = new ICpuComServiceListener.Stub() {
        @Override
        public void onReceiveCmd(CpuCommand command) {
            mLatch.countDown();
        }
    };
    private final ICpuComServiceErrorListener mErrorListener = new ICpuComServiceErrorListener.Stub() {
        @Override
        public void onError(int i, CpuCommand cpuCommand) {
            mLatch.countDown();
        }
    };

    public boolean run() {
        CpuCommand echoRequest = new CpuCommand(0xfd, 0x01, new byte[128]);
        CpuCommand echoResponse = new CpuCommand(0xfd, 0x81, null);
        boolean timeout = false;
        for (int i = 0; i < mSendsCount; ++i) {
            mLatch = new CountDownLatch(1);
            CpuComManager.getInstance().subscribeErrorCB(mErrorListener);
            CpuComManager.getInstance().subscribeCB(echoResponse, mListener);
            CpuComManager.getInstance().sendCmd(echoRequest);
            try {
                // Timeout was choosen empirically, not the best approach, but good enough for this test.
                timeout = !mLatch.await(5, TimeUnit.SECONDS);
            }
            catch(InterruptedException e) {
            }
            CpuComManager.getInstance().unsubscribeCB(echoResponse, mListener);
            CpuComManager.getInstance().unsubscribeErrorCB(mErrorListener);
        }
        return (timeout == false);
    }
}
