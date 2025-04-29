/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import static android.app.Service.START_STICKY;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.verify;
import static org.powermock.api.mockito.PowerMockito.mockStatic;
import static org.powermock.api.mockito.PowerMockito.verifyZeroInteractions;
import static org.powermock.api.mockito.PowerMockito.when;
import static org.powermock.api.mockito.PowerMockito.whenNew;
import static org.powermock.api.support.membermodification.MemberMatcher.everythingDeclaredIn;
import static org.powermock.api.support.membermodification.MemberMatcher.method;
import static org.powermock.api.support.membermodification.MemberModifier.suppress;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import com.mitsubishielectric.ahu.efw.lib.common.Const;
import com.mitsubishielectric.ahu.efw.lib.extendedservicemanager.ExtSrvManager;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import com.mitsubishielectric.ahu.efw.lib.vehiclepwrmgr.IVehiclePowerServiceListener;
import com.mitsubishielectric.ahu.efw.lib.vehiclepwrmgr.VehiclePowerServiceManager;
import java.io.FileDescriptor;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

@RunWith(PowerMockRunner.class)
@SuppressStaticInitializationFor({
    "com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog",
    "com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager"
})
@PrepareForTest({
    CpuComServiceLog.class,
    CpuComServiceLog.LogID.class,
    MLog.class,
    ExtSrvManager.class,
    VehiclePowerServiceManager.class,
    Bundle.class,
    CpuComService.class
})
public class CpuComServiceTest {
    @Mock private Intent mIntentMock;

    @Mock private Bundle mBundleMock;

    @Mock private IBinder mIBinderMock;

    @Mock private Context mContextMock;

    @Mock private ComponentName mComponentNameMock;

    @Mock private ExtSrvManager mExtSrvManagerMock;

    @Mock private Client mAnonymousClientMock;

    @Mock private VehiclePowerServiceManager mVehiclePowerServiceManagerMock;

    @Mock private IVehiclePowerServiceListener mVehiclePowerServiceListenerMock;

    @Mock private CpuComServiceImpl mServiceImpl;

    @InjectMocks private CpuComService mService;

    private CpuComBroadcastReceiver mCpuComBroadcastReceiver;
    private VehiclePowerServiceListener mVehiclePowerServiceListener;
    private CpuComServiceConnection mServiceConnection;

    @Before
    public void setUp() throws Exception {
        mockStatic(ExtSrvManager.class);
        mockStatic(VehiclePowerServiceManager.class);

        suppress(everythingDeclaredIn(CpuComServiceLog.class));
        suppress(everythingDeclaredIn(CpuComServiceLog.LogID.class));
        suppress(everythingDeclaredIn(MLog.class));

        suppress(method(MLog.class, "w"));
        suppress(method(MLog.class, "d"));

        whenNew(Intent.class).withAnyArguments().thenReturn(mIntentMock);
        whenNew(Client.class).withAnyArguments().thenReturn(mAnonymousClientMock);

        Whitebox.setInternalState(MLog.class, "CPUCOM_SERVICE_FUNCTION_ID", 0);
        Whitebox.setInternalState(mService, "mImpl", mServiceImpl);
        Whitebox.setInternalState(
                mService, "mVehiclePowerServiceListener", mVehiclePowerServiceListenerMock);

        when(mIntentMock.getExtras()).thenReturn(mBundleMock);
        when(ExtSrvManager.class, "getInstance").thenReturn(mExtSrvManagerMock);
        when(VehiclePowerServiceManager.class, "getInstance")
                .thenReturn(mVehiclePowerServiceManagerMock);

        mCpuComBroadcastReceiver = new CpuComBroadcastReceiver(mServiceImpl);
        mVehiclePowerServiceListener = new VehiclePowerServiceListener();
        mServiceConnection = new CpuComServiceConnection(mService);
    }

    @Test
    public void createDefaultInstance() {
        new CpuComService();
    }

    @Test
    public void createInstance() {
        new CpuComService(mServiceImpl);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullServiceImpl() {
        new CpuComService(null);
    }

    /** Expected result: There are no exceptions. */
    @Test
    public void serviceOnCreate() {
        mService.onCreate();

        verify(mServiceImpl).initialize();
    }

    /** Expected result: There are no exceptions. */
    @Test
    public void serviceOnDestroy() {
        mService.onDestroy();
        verify(mServiceImpl).terminate();
    }

    /** Expected result: There are no exceptions. */
    @Test
    public void serviceOnBind() {
        assertNull(mService.onBind(new Intent()));
    }

    /** Expected result: There are no exceptions. */
    @Test
    public void serviceOnStartCommand() {
        when(mIntentMock.getExtras()).thenReturn(mBundleMock);
        when(mBundleMock.getBinder(Const.ESM_SERVICE)).thenReturn(mIBinderMock);

        assertEquals(START_STICKY, mService.onStartCommand(mIntentMock, 0, 0));

        verify(mExtSrvManagerMock).addService(Const.CPU_COM_SERVICE, mServiceImpl);
    }

    @Test
    public void serviceOnStartCommandWithException() throws RemoteException {
        when(mIntentMock.getExtras()).thenReturn(mBundleMock);
        when(mBundleMock.getBinder(Const.ESM_SERVICE)).thenReturn(mIBinderMock);

        doThrow(RemoteException.class)
                .when(mVehiclePowerServiceManagerMock)
                .subscribeFWService(eq(mVehiclePowerServiceListenerMock), anyString());

        assertEquals(START_STICKY, mService.onStartCommand(mIntentMock, 0, 0));

        verify(mExtSrvManagerMock).addService(Const.CPU_COM_SERVICE, mServiceImpl);
    }

    @Test
    public void serviceOnStartCommandIntentIsNull() {
        mService.onStartCommand(null, 0, 0);

        verify(mIntentMock).setClassName(Const.ESM_PACKAGE, Const.ESM_SERVICE);
    }

    @Test
    public void serviceOnStartCommandIntentExtrasIsNull() {
        when(mIntentMock.getExtras()).thenReturn(null);

        mService.onStartCommand(mIntentMock, 0, 0);

        verify(mIntentMock).setClassName(Const.ESM_PACKAGE, Const.ESM_SERVICE);
    }

    @Test
    public void serviceOnServiceConnected() throws RemoteException {
        mServiceConnection.onServiceConnected(mComponentNameMock, mIBinderMock);

        verify(mExtSrvManagerMock).addService(Const.CPU_COM_SERVICE, mServiceImpl);
        verify(mVehiclePowerServiceManagerMock)
                .subscribeFWService(eq(mVehiclePowerServiceListenerMock), anyString());
    }

    @Test
    public void serviceOnServiceDisconnected() {
        mServiceConnection.onServiceDisconnected(mComponentNameMock);

        verifyZeroInteractions(mExtSrvManagerMock);
        verifyZeroInteractions(mVehiclePowerServiceManagerMock);
    }

    @Test
    public void serviceOnReceive() {
        mCpuComBroadcastReceiver.onReceive(mContextMock, mIntentMock);

        verify(mExtSrvManagerMock).addService(Const.CPU_COM_SERVICE, mServiceImpl);
    }

    @Test
    public void serviceOnAppStart() throws RemoteException {
        mVehiclePowerServiceListener.onAppStart();

        verify(mVehiclePowerServiceManagerMock)
                .startComplete(CpuComService.class.getCanonicalName());
    }

    @Test
    public void serviceOnAppStartWithException() throws RemoteException {
        doThrow(RemoteException.class)
                .when(mVehiclePowerServiceManagerMock)
                .startComplete(anyString());

        mVehiclePowerServiceListener.onAppStart();
    }

    @Test
    public void serviceOnAppRestart() throws RemoteException {
        mVehiclePowerServiceListener.onAppRestart();

        verify(mVehiclePowerServiceManagerMock)
                .restartCompleteEfw(CpuComService.class.getCanonicalName());
    }

    @Test
    public void serviceOnAppRestartWithException() throws RemoteException {
        doThrow(RemoteException.class)
                .when(mVehiclePowerServiceManagerMock)
                .restartCompleteEfw(anyString());

        mVehiclePowerServiceListener.onAppRestart();
    }

    @Test
    public void serviceOnAppStop() throws RemoteException {
        mVehiclePowerServiceListener.onAppStop();

        verify(mVehiclePowerServiceManagerMock)
                .stopCompleteEfw(CpuComService.class.getCanonicalName());
    }

    @Test
    public void serviceOnAppStopWithException() throws RemoteException {
        doThrow(RemoteException.class)
                .when(mVehiclePowerServiceManagerMock)
                .stopCompleteEfw(anyString());

        mVehiclePowerServiceListener.onAppStop();
    }

    @Test
    public void serviceOnAppResume() throws RemoteException {
        mVehiclePowerServiceListener.onAppResume();

        verify(mVehiclePowerServiceManagerMock)
                .resumeCompleteEfw(CpuComService.class.getCanonicalName());
    }

    @Test
    public void serviceOnAppResumeWithException() throws RemoteException {
        doThrow(RemoteException.class)
                .when(mVehiclePowerServiceManagerMock)
                .resumeCompleteEfw(anyString());

        mVehiclePowerServiceListener.onAppResume();
    }

    /** Expected result: There are no exceptions. */
    @Test
    public void serviceOnDump() {
        mService.dump(
                new FileDescriptor(),
                new PrintWriter(
                        new OutputStream() {
                            @Override
                            public void write(int b) throws IOException {}
                        }),
                null);
    }
}
