/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

import com.mitsubishielectric.ahu.efw.cpucomservice.Client;
import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.concurrent.ExecutorService;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

@RunWith(PowerMockRunner.class)
@SuppressStaticInitializationFor("com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager")
@PrepareForTest(MLog.class)
public class SetErrorCallbackTaskTest {

    private ClientManager mClientManagerMock = mock(ClientManager.class);
    private Client mClientMock = mock(Client.class);
    private ExecutorService mThreadPoolMock = mock(ExecutorService.class);
    private final int mPid = 123;
    private ICpuComServiceErrorListener mErrorCallbackMock =
            mock(ICpuComServiceErrorListener.class);

    private final SetErrorCallbackTask mSetErrorCallbackTask;

    public SetErrorCallbackTaskTest() {
        mockStatic(MLog.class);
        Whitebox.setInternalState(MLog.class, "CPUCOM_SERVICE_FUNCTION_ID", 0);
        mSetErrorCallbackTask =
                new SetErrorCallbackTask(
                        mClientManagerMock, mThreadPoolMock, mPid, mErrorCallbackMock);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullClientManager() {
        new SetErrorCallbackTask(null, mThreadPoolMock, mPid, mErrorCallbackMock);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullThreadPool() {
        new SetErrorCallbackTask(mClientManagerMock, null, mPid, mErrorCallbackMock);
    }

    @Test
    public void runWithNotNullClient() {
        when(mClientManagerMock.getClient(mPid)).thenReturn(mClientMock);
        when(mErrorCallbackMock.asBinder()).thenReturn(null);

        mSetErrorCallbackTask.run();

        verify(mClientMock).setErrorCallback(mErrorCallbackMock);
    }

    @Test
    public void runWithNullClient() {
        when(mClientManagerMock.getClient(mPid)).thenReturn(null);
        when(mErrorCallbackMock.asBinder()).thenReturn(null);
        when(mClientManagerMock.createClient(eq(mPid), eq(null), any(ClientDeathObserver.class)))
                .thenReturn(mClientMock);

        mSetErrorCallbackTask.run();

        verify(mClientMock).setErrorCallback(mErrorCallbackMock);
    }

    @Test
    public void runWithNullClientAndCreateClientReturnNull() {
        when(mClientManagerMock.getClient(mPid)).thenReturn(null);
        when(mErrorCallbackMock.asBinder()).thenReturn(null);
        when(mClientManagerMock.createClient(eq(mPid), eq(null), any(ClientDeathObserver.class)))
                .thenReturn(null);

        mSetErrorCallbackTask.run();

        verify(mClientMock, never()).setErrorCallback(mErrorCallbackMock);
    }
}
