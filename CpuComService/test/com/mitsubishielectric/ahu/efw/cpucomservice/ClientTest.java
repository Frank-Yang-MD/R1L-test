/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.nullable;
import static org.mockito.Mockito.verify;
import static org.powermock.api.mockito.PowerMockito.doCallRealMethod;
import static org.powermock.api.mockito.PowerMockito.verifyPrivate;
import static org.powermock.api.mockito.PowerMockito.verifyZeroInteractions;
import static org.powermock.api.mockito.PowerMockito.when;
import static org.powermock.api.support.membermodification.MemberMatcher.method;
import static org.powermock.api.support.membermodification.MemberModifier.suppress;

import android.os.IBinder;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.Function;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.stubbing.Answer;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

@RunWith(PowerMockRunner.class)
@PrepareForTest({Client.class})
public class ClientTest {
    @Mock private IBinder mIBinderMock;

    @Mock private ICpuComServiceListener mCpuComServiceListenerMock;

    @Mock private Map<CpuCommand, NativeCallbackWrapper> mCallbacksMock;

    @Mock private NativeErrorCallbackWrapper mErrorCallbackMock;

    @Mock private NativeCallbackWrapper mCallbackWrapperMock;

    @Mock private CpuCommand mCpuCommandMock;

    private static final int mPid = 123;
    private Client mClient;

    @Before
    public void setUp() {
        suppress(method(Client.class, "create"));
        suppress(method(Client.class, "destroy"));
        suppress(method(Client.class, "subscribe"));
        suppress(method(Client.class, "unsubscribe"));
        suppress(method(Client.class, "send", int.class, int.class, int.class, byte[].class));
        suppress(method(Client.class, "setErrorCallback", int.class, Object.class));

        mClient = new Client(mPid, mIBinderMock);

        Whitebox.setInternalState(mClient, "mBinder", mIBinderMock);
        Whitebox.setInternalState(mClient, "mCallbacks", mCallbacksMock);
        Whitebox.setInternalState(mClient, "mErrorCallback", mErrorCallbackMock);
    }

    @Test
    public void getPid() {
        assertEquals(mPid, mClient.getPid());
    }

    @Test
    public void getBinder() {
        assertEquals(mIBinderMock, mClient.getBinder());
    }

    @Test
    public void addListenerWrapperEmpty() {
        when(mCallbacksMock.computeIfAbsent(any(CpuCommand.class), any(Function.class)))
                .thenReturn(mCallbackWrapperMock);
        when(mCallbackWrapperMock.isEmpty()).thenReturn(true);

        mClient.addListener(mCpuCommandMock, mCpuComServiceListenerMock);

        verify(mCallbacksMock).computeIfAbsent(any(CpuCommand.class), any(Function.class));
        verify(mCallbackWrapperMock).put(any(ICpuComServiceListener.class));
    }

    @Test
    public void addListenerWrapperNotEmpty() {
        when(mCallbacksMock.computeIfAbsent(any(CpuCommand.class), any(Function.class)))
                .thenReturn(mCallbackWrapperMock);
        when(mCallbackWrapperMock.isEmpty()).thenReturn(false);

        mClient.addListener(mCpuCommandMock, mCpuComServiceListenerMock);

        verify(mCallbacksMock).computeIfAbsent(any(CpuCommand.class), any(Function.class));
        verify(mCallbackWrapperMock).put(any(ICpuComServiceListener.class));
    }

    @Test(expected = NullPointerException.class)
    public void addListenerWithNullCommand() {
        mClient.addListener(null, mCpuComServiceListenerMock);
    }

    @Test(expected = NullPointerException.class)
    public void addListenerWithNullListener() {
        mClient.addListener(mCpuCommandMock, null);
    }

    @Test
    public void addListenerCallbackAbsent() {
        when(mCallbackWrapperMock.isEmpty()).thenReturn(true);

        final AtomicReference<Function<CpuCommand, NativeCallbackWrapper>> reference =
                new AtomicReference<>();

        when(mCallbacksMock.computeIfAbsent(any(CpuCommand.class), any(Function.class)))
                .thenAnswer(
                        (Answer<NativeCallbackWrapper>)
                                invocation -> {
                                    final Function<CpuCommand, NativeCallbackWrapper> function =
                                            (Function<CpuCommand, NativeCallbackWrapper>)
                                                    invocation.getArgument(1);
                                    reference.set(function);

                                    return mCallbackWrapperMock;
                                });

        mClient.addListener(mCpuCommandMock, mCpuComServiceListenerMock);
        reference.get().apply(mCpuCommandMock);

        verify(mCallbacksMock).computeIfAbsent(any(CpuCommand.class), any(Function.class));
    }

    @Test
    public void removeListenerWithNullWrapper() {
        mClient.removeListener(mCpuCommandMock, mCpuComServiceListenerMock);

        verify(mCallbacksMock).get(any(CpuCommand.class));
    }

    @Test
    public void removeListenerWithNotNullAndNotEmptyWrapper() {
        when(mCallbacksMock.get(any(CpuCommand.class))).thenReturn(mCallbackWrapperMock);

        mClient.removeListener(mCpuCommandMock, mCpuComServiceListenerMock);

        verify(mCallbackWrapperMock).remove(any(ICpuComServiceListener.class));
    }

    @Test
    public void removeListenerWithNotNullAndEmptyWrapper() {
        when(mCallbacksMock.get(any(CpuCommand.class))).thenReturn(mCallbackWrapperMock);
        when(mCallbackWrapperMock.isEmpty()).thenReturn(true);

        mClient.removeListener(mCpuCommandMock, mCpuComServiceListenerMock);

        verify(mCallbackWrapperMock).remove(any(ICpuComServiceListener.class));
        verify(mCallbacksMock).remove(any(CpuCommand.class));
    }

    @Test
    public void hasAliveRemoteCallbacks() {
        mClient.hasAliveRemoteCallbacks();

        verify(mCallbacksMock).isEmpty();
    }

    @Test
    public void hasAliveRemoteCallbacksWithEmptyCallbacks() {
        when(mCallbacksMock.isEmpty()).thenReturn(true);

        mClient.hasAliveRemoteCallbacks();

        verify(mCallbacksMock).isEmpty();
    }

    @Test
    public void hasAliveRemoteCallbacksWithNullCallbacks() {
        when(mCallbacksMock.isEmpty()).thenReturn(true);

        Whitebox.setInternalState(mClient, "mErrorCallback", (NativeErrorCallbackWrapper) null);
        mClient.hasAliveRemoteCallbacks();

        verify(mCallbacksMock).isEmpty();
    }

    @RunWith(PowerMockRunner.class)
    @PrepareForTest({Client.class})
    public static class ClientNativeTest {
        @Mock private ICpuComServiceErrorListener mCpuComServiceErrorListenerMock;

        @Mock private NativeErrorCallbackWrapper mErrorCallbackMock;

        @Mock private IBinder mIBinderMock;

        @Mock private CpuCommand mCpuCommandMock;

        @Mock private Client mClientMock;

        @Before
        public void setUp() {
            Whitebox.setInternalState(mClientMock, "mPid", mPid);
            Whitebox.setInternalState(mClientMock, "mBinder", mIBinderMock);
            Whitebox.setInternalState(mClientMock, "mErrorCallback", mErrorCallbackMock);
        }

        @Test
        public void destroy() throws Exception {
            doCallRealMethod().when(mClientMock, "destroy");

            mClientMock.destroy();

            verifyPrivate(mClientMock).invoke("destroy", anyInt());
        }

        @Test
        public void send() throws Exception {
            doCallRealMethod().when(mClientMock, "send", any(CpuCommand.class));

            mClientMock.send(mCpuCommandMock);

            verifyPrivate(mClientMock).invoke("send", anyInt(), anyInt(), anyInt(), any());
        }

        @Test
        public void setErrorCallback() throws Exception {
            doCallRealMethod()
                    .when(mClientMock, "setErrorCallback", any(ICpuComServiceErrorListener.class));

            mClientMock.setErrorCallback(mCpuComServiceErrorListenerMock);

            verifyPrivate(mClientMock).invoke("setErrorCallback", anyInt(), any(Object.class));
            verify(mErrorCallbackMock).destroy();
        }

        @Test
        public void setErrorCallbackWithNullListener() throws Exception {
            doCallRealMethod().when(mClientMock, "setErrorCallback", any());

            Whitebox.setInternalState(
                    mClientMock, "mErrorCallback", (NativeErrorCallbackWrapper) null);
            mClientMock.setErrorCallback((ICpuComServiceErrorListener) null);

            verifyPrivate(mClientMock)
                    .invoke(
                            "setErrorCallback",
                            anyInt(),
                            nullable(NativeErrorCallbackWrapper.class));
            verifyZeroInteractions(mErrorCallbackMock);
        }
    }
}
