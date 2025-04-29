package com.mitsubishielectric.ahu.efw.cpucomservice;

import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.mockStatic;
import static org.powermock.api.support.membermodification.MemberMatcher.method;
import static org.powermock.api.support.membermodification.MemberModifier.suppress;

import android.content.Context;
import android.content.pm.PackageManager;
import com.google.common.util.concurrent.MoreExecutors;
import com.mitsubishielectric.ahu.efw.cpucomservice.permissionchecker.IPermissionChecker;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ExecutorService;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

@RunWith(PowerMockRunner.class)
@SuppressStaticInitializationFor({
    "com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog",
    "com.mitsubishielectric.ahu.efw.cpucomservice.ClientManager",
    "com.mitsubishielectric.ahu.efw.cpucomservice.Client"
})
public class CpuComServiceImplTest {
    @Mock private ExecutorService mThreadPoolMock;

    @Mock private ClientManager mClientManager;

    @Mock private Client mClient;

    @Mock private ICpuComServiceListener mListener;

    @Mock private ICpuComServiceErrorListener mErrorListener;

    @Mock private IPermissionChecker mPermissionCheckerMock;

    private Context mContextMock = null;

    @Mock private CpuComServiceImpl mCpuComService;

    private static final CpuCommand TEST_COMMAND1 = new CpuCommand(1, 1, new byte[1]);
    private static final CpuCommand TEST_COMMAND2 = new CpuCommand(2, 2, new byte[2]);

    @Before
    public void setUp() {
        suppress(method(Client.class, "create"));

        mockStatic(MLog.class);
        Whitebox.setInternalState(MLog.class, "CPUCOM_SERVICE_FUNCTION_ID", 0);

        when(mClientManager.getClient(anyInt())).thenReturn(mClient);

        mCpuComService =
                new CpuComServiceImpl(
                        MoreExecutors.newDirectExecutorService(),
                        mClientManager,
                        mPermissionCheckerMock,
                        mContextMock);
    }

    @Test
    public void createDefaultInstance() {
        new CpuComServiceImpl();
    }

    @Test
    public void initialize() {
        assertTrue(mCpuComService.initialize());
    }

    @Test
    public void terminate() {
        Whitebox.setInternalState(mCpuComService, "mThreadPool", mThreadPoolMock);

        mCpuComService.terminate();

        verify(mThreadPoolMock).shutdown();
    }

    @Test
    public void testSendCmd() {
        when(mPermissionCheckerMock.isAccessGranted(any())).thenReturn(true);

        mCpuComService.sendCmd(TEST_COMMAND1);
        verify(mClient).send(TEST_COMMAND1);
    }

    @Test
    public void testSendCmdWithNonNullContext() {
        mContextMock = mock(Context.class);
        final PackageManager packageManagerMock = mock(PackageManager.class);

        Whitebox.setInternalState(mCpuComService, "mContext", mContextMock);

        when(mContextMock.getPackageManager()).thenReturn(packageManagerMock);
        when(packageManagerMock.getNameForUid(anyInt())).thenReturn("");

        when(mPermissionCheckerMock.isAccessGranted(any())).thenReturn(true);

        mCpuComService.sendCmd(TEST_COMMAND2);
        verify(mClient).send(TEST_COMMAND2);
    }

    @Test
    public void testSendCmdWhenAccessIsDenied() {
        when(mPermissionCheckerMock.isAccessGranted(any())).thenReturn(false);

        mCpuComService.sendCmd(TEST_COMMAND1);
        verifyZeroInteractions(mClient);
    }

    @Test
    public void testSubscribeCB() {
        when(mPermissionCheckerMock.isAccessGranted(any())).thenReturn(true);

        mCpuComService.subscribeCB(TEST_COMMAND1, mListener);
        verify(mClient).addListener(TEST_COMMAND1, mListener);
    }

    @Test
    public void testSubscribeCBWhenAccessIsDenied() {
        when(mPermissionCheckerMock.isAccessGranted(any())).thenReturn(false);

        mCpuComService.subscribeCB(TEST_COMMAND1, mListener);
        verifyZeroInteractions(mClient);
    }

    @Test
    public void testUnsubscribeCB() {
        mCpuComService.unsubscribeCB(TEST_COMMAND1, mListener);
        verify(mClient).removeListener(TEST_COMMAND1, mListener);
    }

    @Test
    public void subscribeErrorCB() {
        mCpuComService.subscribeErrorCB(mErrorListener);
        verify(mClient).setErrorCallback(mErrorListener);
    }

    @Test
    public void unsubscribeErrorCB() {
        mCpuComService.unsubscribeErrorCB(mErrorListener);
        verify(mClient).setErrorCallback(null);
    }

    @Test
    public void subscribeListCB() {
        when(mPermissionCheckerMock.isAccessGranted(any())).thenReturn(true);

        List<CpuCommand> commands = Arrays.asList(TEST_COMMAND1, TEST_COMMAND2);
        mCpuComService.subscribeListCB(commands, mListener);
        verify(mClient).addListener(TEST_COMMAND1, mListener);
        verify(mClient).addListener(TEST_COMMAND2, mListener);
    }

    @Test
    public void subscribeListCBWhenAccessIsDenied() {
        when(mPermissionCheckerMock.isAccessGranted(any())).thenReturn(false);

        List<CpuCommand> commands = Arrays.asList(TEST_COMMAND1, TEST_COMMAND2);
        mCpuComService.subscribeListCB(commands, mListener);

        verifyZeroInteractions(mClient);
    }

    @Test
    public void unsubscribeListCB() {
        List<CpuCommand> commands = Arrays.asList(TEST_COMMAND1, TEST_COMMAND2);
        mCpuComService.unsubscribeListCB(commands, mListener);
        verify(mClient).removeListener(TEST_COMMAND1, mListener);
        verify(mClient).removeListener(TEST_COMMAND2, mListener);
    }
}
