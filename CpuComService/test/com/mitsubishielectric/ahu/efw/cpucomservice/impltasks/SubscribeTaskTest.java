/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import static org.junit.Assert.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

import com.mitsubishielectric.ahu.efw.cpucomservice.Client;
import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.concurrent.ExecutorService;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

@RunWith(PowerMockRunner.class)
@SuppressStaticInitializationFor({
    "com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog",
    "com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager"
})
@PrepareForTest({MLog.class})
public class SubscribeTaskTest {

    private ClientManager mClientManagerMock = mock(ClientManager.class);
    private Client mClientMock = mock(Client.class);
    private ExecutorService mThreadPoolMock = mock(ExecutorService.class);
    private final int mPid = 123;
    private CpuCommand mCommandMock = mock(CpuCommand.class);
    private ICpuComServiceListener mListenerMock = mock(ICpuComServiceListener.class);

    private final SubscribeTask mSubscribeTask;

    public SubscribeTaskTest() {
        mockStatic(MLog.class);
        Whitebox.setInternalState(MLog.class, "CPUCOM_SERVICE_FUNCTION_ID", 0);
        mSubscribeTask =
                new SubscribeTask(
                        mClientManagerMock, mThreadPoolMock, mPid, mCommandMock, mListenerMock);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullClientManager() {
        new SubscribeTask(null, mThreadPoolMock, mPid, mCommandMock, mListenerMock);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullThreadPool() {
        new SubscribeTask(mClientManagerMock, null, mPid, mCommandMock, mListenerMock);
    }

    @Test
    public void runWithInvalidCommand() {
        when(mCommandMock.isValid()).thenReturn(false);

        mSubscribeTask.run();

        verifyZeroInteractions(mClientManagerMock, mClientMock, mListenerMock, mThreadPoolMock);
    }

    @Test
    public void runWithClientNotNull() {
        when(mCommandMock.isValid()).thenReturn(true);
        when(mClientManagerMock.getClient(mPid)).thenReturn(mClientMock);

        mSubscribeTask.run();

        verify(mClientMock).addListener(mCommandMock, mListenerMock);
    }

    @Test
    public void runWithClientIsNullAndCreateClientIsNotNull() {
        when(mCommandMock.isValid()).thenReturn(true);
        when(mClientManagerMock.getClient(mPid)).thenReturn(null);
        when(mListenerMock.asBinder()).thenReturn(null);
        when(mClientManagerMock.createClient(eq(mPid), eq(null), any())).thenReturn(mClientMock);

        mSubscribeTask.run();

        verify(mClientMock).addListener(mCommandMock, mListenerMock);
    }

    @Test
    public void runWithClientIsNullAndCreateClientIsNull() {
        when(mCommandMock.isValid()).thenReturn(true);
        when(mClientManagerMock.getClient(mPid)).thenReturn(null);
        when(mListenerMock.asBinder()).thenReturn(null);
        when(mClientManagerMock.createClient(eq(mPid), eq(null), any())).thenReturn(null);

        mSubscribeTask.run();

        verify(mClientMock, never()).addListener(eq(mCommandMock), any());
    }
}
