/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

import com.mitsubishielectric.ahu.efw.cpucomservice.Client;
import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

@RunWith(PowerMockRunner.class)
@SuppressStaticInitializationFor("com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager")
@PrepareForTest({MLog.class})
public class SendCommandTaskTest {

    private ClientManager mClientManagerMock = mock(ClientManager.class);
    private CpuCommand mCpuCommandMock = mock(CpuCommand.class);
    private Client mClientMock = mock(Client.class);

    private final int mPid = 123;

    private SendCommandTask mSendCommandTask;

    public SendCommandTaskTest() {
        mockStatic(MLog.class);
        Whitebox.setInternalState(MLog.class, "CPUCOM_SERVICE_FUNCTION_ID", 0);
        mSendCommandTask = new SendCommandTask(mClientManagerMock, mPid, mCpuCommandMock);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullClientManager() {
        new SendCommandTask(null, mPid, mCpuCommandMock);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullCpuCommand() {
        new SendCommandTask(mClientManagerMock, mPid, null);
    }

    @Test
    public void runWithInvalidTask() {
        when(mCpuCommandMock.isValid()).thenReturn(false);

        mSendCommandTask.run();

        verifyZeroInteractions(mClientManagerMock);
    }

    @Test
    public void runWithAnonymousClient() {
        when(mCpuCommandMock.isValid()).thenReturn(true);
        when(mClientManagerMock.getClient(mPid)).thenReturn(null);
        when(mClientManagerMock.getAnonymousClient()).thenReturn(mClientMock);

        mSendCommandTask.run();

        verify(mClientMock).send(mCpuCommandMock);
    }

    @Test
    public void runWithInitializedClient() {
        when(mCpuCommandMock.isValid()).thenReturn(true);
        when(mClientManagerMock.getClient(mPid)).thenReturn(mClientMock);

        mSendCommandTask.run();

        verify(mClientMock).send(any());
    }
}
