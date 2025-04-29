package com.mitsubishielectric.ahu.efw.cpucomservice.permissionchecker;

import static org.junit.Assert.*;

import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import org.junit.Test;

public class PermissionCheckerDefaultTest {

    private final PermissionCheckerDefault mPermissionChecker = new PermissionCheckerDefault();

    @Test
    public void isAccessGranted() {
        CpuCommand cpuCommand = new CpuCommand(0, 0, new byte[1]);
        assertTrue(mPermissionChecker.isAccessGranted(cpuCommand));
    }
}
