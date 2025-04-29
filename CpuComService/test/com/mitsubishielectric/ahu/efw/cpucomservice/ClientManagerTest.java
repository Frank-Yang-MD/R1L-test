/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.whenNew;

import android.os.IBinder;
import android.os.RemoteException;
import java.util.List;
import java.util.Map;
import java.util.stream.Stream;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

@RunWith(PowerMockRunner.class)
@SuppressStaticInitializationFor("com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager")
public class ClientManagerTest {
    @Mock private IBinder mIBinderMock;

    @Mock private IBinder.DeathRecipient mDeathRecipientMock;

    @Mock private List<Client> mClientsMock;

    @Mock private Client mClientMock;

    @Mock private Client mAnonymousClientMock;

    @Mock private Map<IBinder, IBinder.DeathRecipient> mDeathObserversMock;

    private ClientManager mClientManager;
    private final int mPid = 123;

    @Before
    public void setUp() throws Exception {
        whenNew(Client.class).withAnyArguments().thenReturn(mAnonymousClientMock);

        mClientManager = new ClientManager();

        Whitebox.setInternalState(mClientManager, "mClients", mClientsMock);
        Whitebox.setInternalState(mClientManager, "mAnonymousClient", mAnonymousClientMock);
        Whitebox.setInternalState(mClientManager, "mDeathObservers", mDeathObserversMock);
    }

    @Test
    public void getClient() {
        when(mClientsMock.stream()).thenReturn(Stream.of(mClientMock));
        when(mClientMock.getPid()).thenReturn(mPid);

        assertEquals(mClientMock, mClientManager.getClient(mPid));
    }

    @Test
    public void getClientWithNotExistPid() {
        when(mClientsMock.stream()).thenReturn(Stream.of(mClientMock));
        when(mClientMock.getPid()).thenReturn(mPid);

        assertNull(mClientManager.getClient(mPid + 1));
    }

    @Test
    public void getAnonymousClient() {
        assertEquals(mAnonymousClientMock, mClientManager.getAnonymousClient());
    }

    @Test
    public void createClient() throws RemoteException {
        assertNotNull(mClientManager.createClient(mPid, mIBinderMock, mDeathRecipientMock));

        verify(mClientsMock).add(any(Client.class));
        verify(mIBinderMock).linkToDeath(mDeathRecipientMock, 0);
        verify(mDeathObserversMock).put(mIBinderMock, mDeathRecipientMock);
    }

    @Test(expected = NullPointerException.class)
    public void createClientWithNullBinder() {
        mClientManager.createClient(mPid, null, mDeathRecipientMock);
    }

    @Test
    public void createClientWithException() throws RemoteException {
        doThrow(RemoteException.class).when(mIBinderMock).linkToDeath(mDeathRecipientMock, 0);

        assertNull(mClientManager.createClient(mPid, mIBinderMock, mDeathRecipientMock));

        verifyZeroInteractions(mDeathObserversMock);
    }

    @Test
    public void destroyClient() {
        when(mClientMock.getBinder()).thenReturn(mIBinderMock);

        mClientManager.destroyClient(mClientMock);

        verify(mDeathObserversMock).get(any(IBinder.class));
        verify(mDeathObserversMock).remove(any(IBinder.class));
        verify(mClientsMock).remove(any(Client.class));
    }

    @Test(expected = NullPointerException.class)
    public void destroyClientWithNullClient() {
        mClientManager.destroyClient(null);

        verifyZeroInteractions(mDeathObserversMock);
        verifyZeroInteractions(mClientsMock);
    }
}
