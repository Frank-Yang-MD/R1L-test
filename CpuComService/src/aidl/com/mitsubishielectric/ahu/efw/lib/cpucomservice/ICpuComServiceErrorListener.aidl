/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.lib.cpucomservice;

import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;

/**
 * CPU Communication Service error Callback Listener for other application
 */
oneway interface ICpuComServiceErrorListener {

    /**
     * Recieve error callback listener
     */
    void onError(in int errors, in CpuCommand command);
}
