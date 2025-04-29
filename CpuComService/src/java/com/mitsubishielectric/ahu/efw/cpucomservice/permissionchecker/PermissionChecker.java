/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.permissionchecker;

import android.content.Context;
import android.content.pm.PackageManager;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import java.util.Objects;

/**
 * Implementation {@link IPermissionChecker} interface. Check permission according to
 * AndroidManifest.xml
 */
public final class PermissionChecker implements IPermissionChecker {

    private static final String PERMISSION_NAME_PATTERN =
            "com.mitsubishielectric.ahu.efw.cpucomservice.permission.cmd_%02X%02X";

    private final Context mContext;

    public PermissionChecker(Context context) {
        mContext =
                Objects.requireNonNull(
                        context, "CpuCom.PermissionChecker: Context must be not null");
    }

    @Override
    public boolean isAccessGranted(CpuCommand command) {
        return mContext.checkCallingPermission(getPermissionName(command))
                == PackageManager.PERMISSION_GRANTED;
    }

    private String getPermissionName(CpuCommand command) {
        return String.format(PERMISSION_NAME_PATTERN, command.cmd, command.subCmd);
    }
}
