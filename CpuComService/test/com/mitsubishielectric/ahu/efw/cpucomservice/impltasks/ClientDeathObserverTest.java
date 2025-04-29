/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

import com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager;
import java.util.concurrent.ExecutorService;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;

@RunWith(PowerMockRunner.class)
@SuppressStaticInitializationFor("com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager")
public final class ClientDeathObserverTest {
    private ClientManager mClientManagerMock = mock(ClientManager.class);
    private ExecutorService mThreadPoolMock = mock(ExecutorService.class);

    private final int mPid = 123;
    private final ClientDeathObserver mClientDeathObserver;

    public ClientDeathObserverTest() {
        mClientDeathObserver = new ClientDeathObserver(mClientManagerMock, mThreadPoolMock, mPid);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullClientManager() {
        new ClientDeathObserver(null, mThreadPoolMock, mPid);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullThreadPool() {
        new ClientDeathObserver(mClientManagerMock, null, mPid);
    }

    @Test
    public void binderDied() {
        mClientDeathObserver.binderDied();

        verify(mThreadPoolMock).execute(any(DestroyClientTask.class));
    }
}
