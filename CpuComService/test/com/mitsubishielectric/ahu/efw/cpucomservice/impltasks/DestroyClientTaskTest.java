/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import com.mitsubishielectric.ahu.efw.cpucomservice.Client;
import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;

@RunWith(PowerMockRunner.class)
@SuppressStaticInitializationFor("com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager")
@PrepareForTest({MLog.class})
public final class DestroyClientTaskTest {
    private ClientManager mClientManagerMock = mock(ClientManager.class);
    private Client mClientMock = mock(Client.class);

    private final int mPid = 123;
    private final DestroyClientTask mDestroyClientTask;

    public DestroyClientTaskTest() {
        mDestroyClientTask = new DestroyClientTask(mClientManagerMock, mPid);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullClientManager() {
        new DestroyClientTask(null, mPid);
    }

    @Test
    public void runWithInitializedClient() {
        when(mClientManagerMock.getClient(mPid)).thenReturn(mClientMock);

        mDestroyClientTask.run();

        verify(mClientManagerMock).destroyClient(any(Client.class));
    }

    @Test
    public void runWithInitializedNullClient() {
        doReturn(null).when(mClientManagerMock).getClient(mPid);
        doThrow(NullPointerException.class).when(mClientManagerMock).destroyClient(mClientMock);

        mDestroyClientTask.run();
    }
}
