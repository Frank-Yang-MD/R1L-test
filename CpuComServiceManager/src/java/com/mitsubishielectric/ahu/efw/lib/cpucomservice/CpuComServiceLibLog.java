/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.lib.cpucomservice;

import static com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog.LogFormat;
import static java.util.Arrays.asList;

import java.util.List;

public final class CpuComServiceLibLog {

    public static final class LogID {
        public static final Integer ExceptionSendCmd = 19;
        public static final Integer ExceptionSubscribeCB = 20;
        public static final Integer ExceptionSubscribeListCB = 21;
        public static final Integer ExceptionSubscribeErrorCB = 22;
        public static final Integer ExceptionUnsubscribeCB = 23;
        public static final Integer ExceptionUnsubscribeErrorCB = 24;
        public static final Integer ExceptionUnsubscribeListCB = 25;
    }

    protected static final List<LogFormat> logMessageFormats =
            asList(
                    new LogFormat(LogID.ExceptionSendCmd, "sendCmd: Remote exception.\n"),
                    new LogFormat(LogID.ExceptionSubscribeCB, "subscribeCB: Remote exception.\n"),
                    new LogFormat(
                            LogID.ExceptionSubscribeListCB, "subscribeListCB: Remote exception.\n"),
                    new LogFormat(
                            LogID.ExceptionSubscribeErrorCB,
                            "subscribeErrorCB: Remote exception.\n"),
                    new LogFormat(
                            LogID.ExceptionUnsubscribeCB, "unsubscribeCB: Remote exception.\n"),
                    new LogFormat(
                            LogID.ExceptionUnsubscribeErrorCB,
                            "unsubscribeErrorCB: Remote exception.\n"),
                    new LogFormat(
                            LogID.ExceptionUnsubscribeListCB,
                            "unsubscribeListCB: Remote exception.\n"));
}
