/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

import com.mitsubishielectric.ahu.efw.cpucomservice.Client;
import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

@RunWith(PowerMockRunner.class)
@SuppressStaticInitializationFor({
    "com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager",
    "com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog"
})
public class ResetErrorCallbackTaskTest {

    private ClientManager mClientManagerMock = mock(ClientManager.class);
    private Client mClientMock = mock(Client.class);

    private final int mPid = 123;

    private ResetErrorCallbackTask mResetErrorCallbackTask;

    public ResetErrorCallbackTaskTest() {
        mockStatic(MLog.class);
        Whitebox.setInternalState(MLog.class, "CPUCOM_SERVICE_FUNCTION_ID", 0);
        mResetErrorCallbackTask = new ResetErrorCallbackTask(mClientManagerMock, mPid);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullClientManager() {
        new ResetErrorCallbackTask(null, mPid);
    }

    @Test
    public void runWithNullClient() {
        when(mClientManagerMock.getClient(mPid)).thenReturn(null);

        mResetErrorCallbackTask.run();

        verifyZeroInteractions(mClientMock);
    }

    @Test
    public void runWithHasAliveRemoteCallbacksIsFalse() {
        when(mClientMock.hasAliveRemoteCallbacks()).thenReturn(false);
        when(mClientManagerMock.getClient(mPid)).thenReturn(mClientMock);

        mResetErrorCallbackTask.run();

        verify(mClientMock).setErrorCallback(null);
        verify(mClientManagerMock).destroyClient(mClientMock);
    }

    @Test
    public void runWithHasAliveRemoteCallbacksIsTrue() {
        when(mClientMock.hasAliveRemoteCallbacks()).thenReturn(true);
        when(mClientManagerMock.getClient(mPid)).thenReturn(mClientMock);

        mResetErrorCallbackTask.run();

        verify(mClientMock).setErrorCallback(null);
        verify(mClientManagerMock, never()).destroyClient(mClientMock);
    }
}
