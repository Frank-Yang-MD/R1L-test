/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.permissionchecker;

import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;

/**
 * Base interface for the permission checker objects. This object using for grant or decline access
 * to public API
 */
public interface IPermissionChecker {

    /** Check if access is granted */
    boolean isAccessGranted(CpuCommand command);
}
