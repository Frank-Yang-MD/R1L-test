/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

import android.os.RemoteCallbackList;
import android.os.RemoteException;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

@RunWith(PowerMockRunner.class)
@SuppressStaticInitializationFor({"com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog"})
@PrepareForTest({MLog.class})
public class NativeCallbackWrapperTest {

    @Mock private RemoteCallbackList<ICpuComServiceListener> mCallbacksMock;

    @Mock private ICpuComServiceListener mCpuComServiceListenerMock1;
    @Mock private ICpuComServiceListener mCpuComServiceListenerMock2;

    private final int mPid = 1234;

    private final int mCmd = 345;
    private final int mSubCmd = 890;
    private final byte[] mData = {1, 2, 3, 4};
    private final CpuCommand mCpuCommand = new CpuCommand(mCmd, mSubCmd, mData);

    private NativeCallbackWrapper mNativeCallbackWrapper;

    public NativeCallbackWrapperTest() {
        mockStatic(MLog.class);
        Whitebox.setInternalState(MLog.class, "CPUCOM_SERVICE_FUNCTION_ID", 0);
        MockitoAnnotations.initMocks(this);
        mNativeCallbackWrapper = new NativeCallbackWrapper(mPid, mCallbacksMock);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullListeners() {
        new NativeCallbackWrapper(mPid, null);
    }

    @Test
    public void destroy() {
        mNativeCallbackWrapper.destroy();

        verify(mCallbacksMock).kill();
    }

    @Test
    public void put() {
        mNativeCallbackWrapper.put(mCpuComServiceListenerMock1);

        verify(mCallbacksMock).register(mCpuComServiceListenerMock1);
    }

    @Test
    public void remove() {
        mNativeCallbackWrapper.remove(mCpuComServiceListenerMock1);

        verify(mCallbacksMock).unregister(mCpuComServiceListenerMock1);
    }

    @Test
    public void isEmptyMustReturnTrue() {
        final int emptySize = 0;

        when(mCallbacksMock.getRegisteredCallbackCount()).thenReturn(emptySize);

        assertTrue(mNativeCallbackWrapper.isEmpty());
    }

    @Test
    public void isEmptyMustReturnFalse() {
        final int emptySize = 100;

        when(mCallbacksMock.getRegisteredCallbackCount()).thenReturn(emptySize);

        assertFalse(mNativeCallbackWrapper.isEmpty());
    }

    @Test
    public void onCommand() throws RemoteException {
        final int callbacksSize = 2;

        final int firstListenerPosition = 0;
        final int secondListenerPosition = 1;

        when(mCallbacksMock.beginBroadcast()).thenReturn(callbacksSize);
        when(mCallbacksMock.getBroadcastItem(firstListenerPosition))
                .thenReturn(mCpuComServiceListenerMock1);
        when(mCallbacksMock.getBroadcastItem(secondListenerPosition))
                .thenReturn(mCpuComServiceListenerMock2);

        mNativeCallbackWrapper.onCommand(mCmd, mSubCmd, mData);

        verify(mCpuComServiceListenerMock1).onReceiveCmd(mCpuCommand);
        verify(mCpuComServiceListenerMock2).onReceiveCmd(mCpuCommand);

        verify(mCallbacksMock).finishBroadcast();
    }

    @Test
    public void onCommandWithRemoteException() throws RemoteException {
        final int callbacksSize = 2;

        final int firstListenerPosition = 0;
        final int secondListenerPosition = 1;

        when(mCallbacksMock.beginBroadcast()).thenReturn(callbacksSize);
        when(mCallbacksMock.getBroadcastItem(firstListenerPosition))
                .thenReturn(mCpuComServiceListenerMock1);
        when(mCallbacksMock.getBroadcastItem(secondListenerPosition))
                .thenReturn(mCpuComServiceListenerMock2);
        doThrow(new RemoteException()).when(mCpuComServiceListenerMock1).onReceiveCmd(mCpuCommand);

        mNativeCallbackWrapper.onCommand(mCmd, mSubCmd, mData);

        verify(mCpuComServiceListenerMock1).onReceiveCmd(mCpuCommand);
        verify(mCpuComServiceListenerMock2).onReceiveCmd(mCpuCommand);

        verify(mCallbacksMock).finishBroadcast();
    }
}
