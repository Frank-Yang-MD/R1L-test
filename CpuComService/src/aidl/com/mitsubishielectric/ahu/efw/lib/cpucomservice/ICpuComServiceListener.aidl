/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.lib.cpucomservice;

import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;

/**
 * CPU Communication Service Callback Listener for other application
 */
oneway interface ICpuComServiceListener {

    /**
     * Recieve Command Callback Listener
     */
    void onReceiveCmd(in CpuCommand command);
}

