package com.mitsubishielectric.ahu.efw.cpucomservice;

import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.IBinder;
import java.util.Objects;

/** This class is responsible for connection in CpuComService */
public final class CpuComServiceConnection implements ServiceConnection {

    private final CpuComService mService;

    public CpuComServiceConnection(CpuComService service) {
        mService =
                Objects.requireNonNull(
                        service,
                        "CpuComService.CpuComServiceConnection: CpuComService parameter must be not null");
    }

    @Override
    public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
        mService.registerService(iBinder);
    }

    @Override
    public void onServiceDisconnected(ComponentName componentName) {}
}
