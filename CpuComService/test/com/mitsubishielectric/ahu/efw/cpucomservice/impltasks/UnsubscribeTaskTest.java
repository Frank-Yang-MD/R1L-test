package com.mitsubishielectric.ahu.efw.cpucomservice.impltasks;

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
@SuppressStaticInitializationFor("com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager")
@PrepareForTest({MLog.class})
public class UnsubscribeTaskTest {

    private ClientManager mClientManagerMock = mock(ClientManager.class);
    private Client mClientMock = mock(Client.class);
    private ExecutorService mThreadPoolMock = mock(ExecutorService.class);
    private final int mPid = 123;
    private CpuCommand mCpuCommandMock = mock(CpuCommand.class);
    private ICpuComServiceListener mListenerMock = mock(ICpuComServiceListener.class);

    private final UnsubscribeTask mUnsubscribeTask;

    public UnsubscribeTaskTest() {
        mockStatic(MLog.class);
        Whitebox.setInternalState(MLog.class, "CPUCOM_SERVICE_FUNCTION_ID", 0);
        mUnsubscribeTask =
                new UnsubscribeTask(mClientManagerMock, mPid, mCpuCommandMock, mListenerMock);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullClientManager() {
        new UnsubscribeTask(null, mPid, mCpuCommandMock, mListenerMock);
    }

    @Test
    public void runWithNullClient() {
        when(mClientManagerMock.getClient(mPid)).thenReturn(null);

        mUnsubscribeTask.run();

        verifyZeroInteractions(mClientMock);
    }

    @Test
    public void runtWithClientNotNullAndRemoveListenerIsTrueAndHasAliveRemoteCallbacksIsTrue() {
        when(mClientManagerMock.getClient(mPid)).thenReturn(mClientMock);
        when(mClientMock.removeListener(mCpuCommandMock, mListenerMock)).thenReturn(true);
        when(mClientMock.hasAliveRemoteCallbacks()).thenReturn(true);

        mUnsubscribeTask.run();

        verify(mClientManagerMock, never()).destroyClient(mClientMock);
    }

    @Test
    public void runtWithClientNotNullAndRemoveListenerIsTrueAndHasAliveRemoteCallbacksIsFalse() {
        when(mClientManagerMock.getClient(mPid)).thenReturn(mClientMock);
        when(mClientMock.removeListener(mCpuCommandMock, mListenerMock)).thenReturn(true);
        when(mClientMock.hasAliveRemoteCallbacks()).thenReturn(false);

        mUnsubscribeTask.run();

        verify(mClientManagerMock).destroyClient(mClientMock);
    }

    @Test
    public void runtWithClientNotNullAndRemoveListenerIsFalseAndHasAliveRemoteCallbacksIsTrue() {
        when(mClientManagerMock.getClient(mPid)).thenReturn(mClientMock);
        when(mClientMock.removeListener(mCpuCommandMock, mListenerMock)).thenReturn(false);
        when(mClientMock.hasAliveRemoteCallbacks()).thenReturn(true);

        mUnsubscribeTask.run();

        verify(mClientManagerMock, never()).destroyClient(mClientMock);
    }

    @Test
    public void runtWithClientNotNullAndRemoveListenerIsFalseAndHasAliveRemoteCallbacksIsFalse() {
        when(mClientManagerMock.getClient(mPid)).thenReturn(mClientMock);
        when(mClientMock.removeListener(mCpuCommandMock, mListenerMock)).thenReturn(false);
        when(mClientMock.hasAliveRemoteCallbacks()).thenReturn(false);

        mUnsubscribeTask.run();

        verify(mClientManagerMock).destroyClient(mClientMock);
    }
}
