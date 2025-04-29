/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.permissionchecker;

import static org.junit.Assert.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.pm.PackageManager;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import org.junit.Test;

public class PermissionCheckerTest {
    private static final CpuCommand mCpuCommandGranted = new CpuCommand(0, 0, new byte[1]);
    private static final CpuCommand mCpuCommandDenied = new CpuCommand(0, 0, new byte[1]);

    private Context mContextMock = mock(Context.class);

    private PermissionChecker mPermissionChecker;

    public PermissionCheckerTest() {
        mPermissionChecker = new PermissionChecker(mContextMock);
    }

    @Test(expected = NullPointerException.class)
    public void createInstanceWithNullArg0() {
        new PermissionChecker(null);
    }

    @Test
    public void isAccessGranted() {
        when(mContextMock.checkCallingPermission(any()))
                .thenReturn(PackageManager.PERMISSION_GRANTED);

        assertTrue(mPermissionChecker.isAccessGranted(mCpuCommandGranted));
    }

    @Test
    public void isAccessDenied() {
        when(mContextMock.checkCallingPermission(any()))
                .thenReturn(PackageManager.PERMISSION_DENIED);

        assertFalse(mPermissionChecker.isAccessGranted(mCpuCommandDenied));
    }
}
