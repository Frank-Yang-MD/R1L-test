/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

import android.os.RemoteCallbackList;
import android.os.RemoteException;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
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
public class NativeErrorCallbackWrapperTest {

    @Mock private RemoteCallbackList<ICpuComServiceErrorListener> mCallbacksMock;

    @Mock private ICpuComServiceErrorListener mCpuComServiceErrorListenerMock1;
    @Mock private ICpuComServiceErrorListener mCpuComServiceErrorListenerMock2;

    private NativeErrorCallbackWrapper mNativeErrorCallbackWrapper;

    private final int mPid = 1234;

    private final int mCmd = 345;
    private final int mSubCmd = 890;

    private final int mErrorCode = 123456;

    private final CpuCommand mCpuCommand = new CpuCommand(mCmd, mSubCmd, null);

    public NativeErrorCallbackWrapperTest() {
        mockStatic(MLog.class);
        Whitebox.setInternalState(MLog.class, "CPUCOM_SERVICE_FUNCTION_ID", 0);
        MockitoAnnotations.initMocks(this);
        mNativeErrorCallbackWrapper = new NativeErrorCallbackWrapper(mPid, mCallbacksMock);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullListeners() {
        new NativeErrorCallbackWrapper(mPid, null);
    }

    @Test
    public void destroy() {
        mNativeErrorCallbackWrapper.destroy();

        verify(mCallbacksMock).kill();
    }

    @Test
    public void onError() throws RemoteException {
        final int callbacksSize = 2;

        final int firstListenerPosition = 0;
        final int secondListenerPosition = 1;

        when(mCallbacksMock.beginBroadcast()).thenReturn(callbacksSize);
        when(mCallbacksMock.getBroadcastItem(firstListenerPosition))
                .thenReturn(mCpuComServiceErrorListenerMock1);
        when(mCallbacksMock.getBroadcastItem(secondListenerPosition))
                .thenReturn(mCpuComServiceErrorListenerMock2);

        mNativeErrorCallbackWrapper.onError(mCmd, mSubCmd, mErrorCode);

        verify(mCpuComServiceErrorListenerMock1).onError(mErrorCode, mCpuCommand);
        verify(mCpuComServiceErrorListenerMock2).onError(mErrorCode, mCpuCommand);

        verify(mCallbacksMock).finishBroadcast();
    }

    @Test
    public void onErrorWithRemoteException() throws RemoteException {
        final int callbacksSize = 2;

        final int firstListenerPosition = 0;
        final int secondListenerPosition = 1;

        when(mCallbacksMock.beginBroadcast()).thenReturn(callbacksSize);
        when(mCallbacksMock.getBroadcastItem(firstListenerPosition))
                .thenReturn(mCpuComServiceErrorListenerMock1);
        when(mCallbacksMock.getBroadcastItem(secondListenerPosition))
                .thenReturn(mCpuComServiceErrorListenerMock2);
        doThrow(new RemoteException())
                .when(mCpuComServiceErrorListenerMock1)
                .onError(mErrorCode, mCpuCommand);

        mNativeErrorCallbackWrapper.onError(mCmd, mSubCmd, mErrorCode);

        verify(mCpuComServiceErrorListenerMock1).onError(mErrorCode, mCpuCommand);
        verify(mCpuComServiceErrorListenerMock2).onError(mErrorCode, mCpuCommand);

        verify(mCallbacksMock).finishBroadcast();
    }
}
