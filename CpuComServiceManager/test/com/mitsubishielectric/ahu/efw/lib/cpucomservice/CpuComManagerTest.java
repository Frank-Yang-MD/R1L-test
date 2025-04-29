/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.lib.cpucomservice;

import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.IBinder;
import android.os.RemoteException;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.Collections;
import java.util.List;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

@RunWith(PowerMockRunner.class)
@SuppressStaticInitializationFor({"com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog"})
@PrepareForTest({MLog.class, CpuComManager.class, ICpuComService.Stub.class})
public class CpuComManagerTest {

    private IBinder mBinderMock = mock(IBinder.class);
    private ICpuComService mCpuComServiceMock = mock(ICpuComService.class);

    private final CpuComManager mCpuComManager;

    private CpuCommand mCpuCommandMock = mock(CpuCommand.class);
    private List<CpuCommand> mCpuCommandsListMock = Collections.singletonList(mCpuCommandMock);
    private ICpuComServiceListener mCpuComServiceListenerMock = mock(ICpuComServiceListener.class);
    private ICpuComServiceErrorListener mCpuComServiceErrorListener =
            mock(ICpuComServiceErrorListener.class);

    public CpuComManagerTest() {
        PowerMockito.mockStatic(MLog.class, ICpuComService.Stub.class);
        Whitebox.setInternalState(MLog.class, "CPUCOM_SERVICE_FUNCTION_ID", 10);
        when(ICpuComService.Stub.asInterface(mBinderMock)).thenReturn(mCpuComServiceMock);

        mCpuComManager = CpuComManager.getInstance();
    }

    @Test
    public void setBinderTest() {
        CpuComManager.setBinder(mBinderMock);

        PowerMockito.verifyStatic(ICpuComService.Stub.class, Mockito.times(1));
        ICpuComService.Stub.asInterface(mBinderMock);
    }

    @Test(expected = IllegalStateException.class)
    public void setBinderWithIllegalStateExceptionTest() {
        when(ICpuComService.Stub.asInterface(mBinderMock)).thenReturn(null);
        CpuComManager.setBinder(mBinderMock);

        PowerMockito.verifyStatic(ICpuComService.Stub.class, Mockito.times(1));
        ICpuComService.Stub.asInterface(mBinderMock);
    }

    @Test
    public void sendCmdTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);

        mCpuComManager.sendCmd(mCpuCommandMock);

        verify(mCpuComServiceMock).sendCmd(mCpuCommandMock);
    }

    @Test
    public void sendCmdWithExceptionTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);
        doThrow(new RemoteException()).when(mCpuComServiceMock).sendCmd(mCpuCommandMock);

        mCpuComManager.sendCmd(mCpuCommandMock);

        PowerMockito.verifyStatic(MLog.class, Mockito.times(1));
        MLog.w(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLibLog.LogID.ExceptionSendCmd);
    }

    @Test
    public void subscribeCBTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);

        mCpuComManager.subscribeCB(mCpuCommandMock, mCpuComServiceListenerMock);

        verify(mCpuComServiceMock).subscribeCB(mCpuCommandMock, mCpuComServiceListenerMock);
    }

    @Test
    public void subscribeCBWithExceptionTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);
        doThrow(new RemoteException())
                .when(mCpuComServiceMock)
                .subscribeCB(mCpuCommandMock, mCpuComServiceListenerMock);

        mCpuComManager.subscribeCB(mCpuCommandMock, mCpuComServiceListenerMock);

        PowerMockito.verifyStatic(MLog.class, Mockito.times(1));
        MLog.w(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLibLog.LogID.ExceptionSubscribeCB);
    }

    @Test
    public void subscribeListCBTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);

        mCpuComManager.subscribeListCB(mCpuCommandsListMock, mCpuComServiceListenerMock);

        verify(mCpuComServiceMock)
                .subscribeListCB(mCpuCommandsListMock, mCpuComServiceListenerMock);
    }

    @Test
    public void subscribeListCBWithExceptionTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);
        doThrow(new RemoteException())
                .when(mCpuComServiceMock)
                .subscribeListCB(mCpuCommandsListMock, mCpuComServiceListenerMock);

        mCpuComManager.subscribeListCB(mCpuCommandsListMock, mCpuComServiceListenerMock);

        PowerMockito.verifyStatic(MLog.class, Mockito.times(1));
        MLog.w(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLibLog.LogID.ExceptionSubscribeListCB);
    }

    @Test
    public void subscribeErrorCBTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);

        mCpuComManager.subscribeErrorCB(mCpuComServiceErrorListener);

        verify(mCpuComServiceMock).subscribeErrorCB(mCpuComServiceErrorListener);
    }

    @Test
    public void subscribeErrorCBWithExceptionTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);
        doThrow(new RemoteException())
                .when(mCpuComServiceMock)
                .subscribeErrorCB(mCpuComServiceErrorListener);

        mCpuComManager.subscribeErrorCB(mCpuComServiceErrorListener);

        PowerMockito.verifyStatic(MLog.class, Mockito.times(1));
        MLog.w(
                MLog.CPUCOM_SERVICE_FUNCTION_ID,
                CpuComServiceLibLog.LogID.ExceptionSubscribeErrorCB);
    }

    @Test
    public void unsubscribeCBTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);

        mCpuComManager.unsubscribeCB(mCpuCommandMock, mCpuComServiceListenerMock);

        verify(mCpuComServiceMock).unsubscribeCB(mCpuCommandMock, mCpuComServiceListenerMock);
    }

    @Test
    public void unsubscribeCBWithExceptionTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);
        doThrow(new RemoteException())
                .when(mCpuComServiceMock)
                .unsubscribeCB(mCpuCommandMock, mCpuComServiceListenerMock);

        mCpuComManager.unsubscribeCB(mCpuCommandMock, mCpuComServiceListenerMock);

        PowerMockito.verifyStatic(MLog.class, Mockito.times(1));
        MLog.w(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLibLog.LogID.ExceptionUnsubscribeCB);
    }

    @Test
    public void unsubscribeListCBTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);

        mCpuComManager.unsubscribeListCB(mCpuCommandsListMock, mCpuComServiceListenerMock);

        verify(mCpuComServiceMock)
                .unsubscribeListCB(mCpuCommandsListMock, mCpuComServiceListenerMock);
    }

    @Test
    public void unsubscribeListCBWithExceptionTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);
        doThrow(new RemoteException())
                .when(mCpuComServiceMock)
                .unsubscribeListCB(mCpuCommandsListMock, mCpuComServiceListenerMock);

        mCpuComManager.unsubscribeListCB(mCpuCommandsListMock, mCpuComServiceListenerMock);

        PowerMockito.verifyStatic(MLog.class, Mockito.times(1));
        MLog.w(
                MLog.CPUCOM_SERVICE_FUNCTION_ID,
                CpuComServiceLibLog.LogID.ExceptionUnsubscribeListCB);
    }

    @Test
    public void unsubscribeErrorCBTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);

        mCpuComManager.unsubscribeErrorCB(mCpuComServiceErrorListener);

        verify(mCpuComServiceMock).unsubscribeErrorCB(mCpuComServiceErrorListener);
    }

    @Test
    public void unsubscribeErrorCBWithExceptionTest() throws RemoteException {
        CpuComManager.setBinder(mBinderMock);
        doThrow(new RemoteException())
                .when(mCpuComServiceMock)
                .unsubscribeErrorCB(mCpuComServiceErrorListener);

        mCpuComManager.unsubscribeErrorCB(mCpuComServiceErrorListener);

        PowerMockito.verifyStatic(MLog.class, Mockito.times(1));
        MLog.w(
                MLog.CPUCOM_SERVICE_FUNCTION_ID,
                CpuComServiceLibLog.LogID.ExceptionUnsubscribeErrorCB);
    }

    @Test
    public void testCpuComServiceLibLogInstantiation() {
        new CpuComServiceLibLog();
    }

    @Test
    public void testCpuComServiceLibLogIdInstantiation() {
        new CpuComServiceLibLog.LogID();
    }
}
