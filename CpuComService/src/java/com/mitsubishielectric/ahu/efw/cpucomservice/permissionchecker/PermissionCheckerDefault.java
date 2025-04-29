/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.permissionchecker;

import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;

/** Default implementation of {@link IPermissionChecker} interface. Always permission is granted. */
public final class PermissionCheckerDefault implements IPermissionChecker {

    @Override
    public boolean isAccessGranted(CpuCommand command) {
        return true;
    }
}
