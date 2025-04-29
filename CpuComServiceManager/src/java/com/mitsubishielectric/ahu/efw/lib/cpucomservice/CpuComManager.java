/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.lib.cpucomservice;

import android.os.IBinder;
import android.os.RemoteException;
import com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog;
import java.util.List;

/**
 * CpuComManager. CpuComManager ・IBinder wrapper class of CpuComService.
 *
 * @author Iurii Mysan
 */
public class CpuComManager {
    private static final String TAG = "CpuComManager";
    private static ICpuComService sService;

    /**
     * This static inner class contain singleton instance of CpuComManager. It allows avoid to use
     * any synchronization construct in getInstance() method.
     */
    private static class SingletonHolder {
        private static final CpuComManager sInstance = new CpuComManager();
    }

    /**
     * This method is entry point to get instance of CpuComManager.
     *
     * @return CpuComManager instance.
     */
    public static CpuComManager getInstance() {
        return SingletonHolder.sInstance;
    }

    /**
     * This method set binder interface to CpuComSerive.
     *
     * @param binder This is binder interface to CpuComService.
     * @return CpuComManager instance.
     */
    public static void setBinder(IBinder binder) {
        sService = ICpuComService.Stub.asInterface(binder);
        if (sService == null) {
            throw new IllegalStateException("Failed to find ICpuComService");
        }
    }

    /**
     * This method wrap sendCmd method from CpuComService.
     *
     * @param command This is representation of mcpu-vcpu data transaction.
     * @return Nothing.
     */
    public void sendCmd(CpuCommand command) {
        try {
            sService.sendCmd(command);
        } catch (RemoteException ex) {
            MLog.w(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLibLog.LogID.ExceptionSendCmd);
        }
    }

    /**
     * This method wrap subscribe method from CpuComService.
     *
     * @param command This is representation of mcpu-vcpu data transaction.
     * @param listener This is interface for listen vcpu command.
     * @return Nothing.
     */
    public void subscribeCB(CpuCommand command, ICpuComServiceListener listener) {
        try {
            sService.subscribeCB(command, listener);
        } catch (RemoteException ex) {
            MLog.w(MLog.CPUCOM_SERVICE_FUNCTION_ID, CpuComServiceLibLog.LogID.ExceptionSubscribeCB);
        }
    }

    /**
     * This method wrap subscribeListCB method from CpuComService.
     *
     * @param commands This is representation of mcpu-vcpu data list for transition.
     * @param listener This is interface for listen vcpu command.
     * @return Nothing.
     */
    public void subscribeListCB(List<CpuCommand> commands, ICpuComServiceListener listener) {
        try {
            sService.subscribeListCB(commands, listener);
        } catch (RemoteException ex) {
            MLog.w(
                    MLog.CPUCOM_SERVICE_FUNCTION_ID,
                    CpuComServiceLibLog.LogID.ExceptionSubscribeListCB);
        }
    }

    /**
     * This method wrap subscribeErrorCB method from CpuComService.
     *
     * @param errorListener This is interface for listen vcpu error command.
     * @return Nothing.
     */
    public void subscribeErrorCB(ICpuComServiceErrorListener errorListener) {
        try {
            sService.subscribeErrorCB(errorListener);
        } catch (RemoteException ex) {
            MLog.w(
                    MLog.CPUCOM_SERVICE_FUNCTION_ID,
                    CpuComServiceLibLog.LogID.ExceptionSubscribeErrorCB);
        }
    }

    /**
     * This method wrap unsubscribeCB method from CpuComService.
     *
     * @param command This is representation of mcpu-vcpu data transaction.
     * @param listener This is interface for listen vcpu command.
     * @return Nothing.
     */
    public void unsubscribeCB(CpuCommand command, ICpuComServiceListener listener) {
        try {
            sService.unsubscribeCB(command, listener);
        } catch (RemoteException ex) {
            MLog.w(
                    MLog.CPUCOM_SERVICE_FUNCTION_ID,
                    CpuComServiceLibLog.LogID.ExceptionUnsubscribeCB);
        }
    }

    /**
     * This method wraps unsubscribeErrorCB method from CpuComService.
     *
     * @param errorListener the interface for listening command processing errors
     */
    public void unsubscribeErrorCB(ICpuComServiceErrorListener errorListener) {
        try {
            sService.unsubscribeErrorCB(errorListener);
        } catch (RemoteException ex) {
            MLog.w(
                    MLog.CPUCOM_SERVICE_FUNCTION_ID,
                    CpuComServiceLibLog.LogID.ExceptionUnsubscribeErrorCB);
        }
    }

    /**
     * This method wraps unsubscribeListCB method from CpuComService.
     *
     * @param commands the list of vcpu commands to unsubscribe from
     * @param listener the interface for listening vcpu commands
     */
    public void unsubscribeListCB(List<CpuCommand> commands, ICpuComServiceListener listener) {
        try {
            sService.unsubscribeListCB(commands, listener);
        } catch (RemoteException ex) {
            MLog.w(
                    MLog.CPUCOM_SERVICE_FUNCTION_ID,
                    CpuComServiceLibLog.LogID.ExceptionUnsubscribeListCB);
        }
    }
}
