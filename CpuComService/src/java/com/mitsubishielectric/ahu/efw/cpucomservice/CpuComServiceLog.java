/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import static com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog.DisplayTypeDecInt;
import static com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog.DisplayTypeHexInt;
import static com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog.DisplayTypeString;
import static com.mitsubishielectric.ahu.efw.lib.logdogcommonclasslib.MLog.LogFormat;
import static java.util.Arrays.asList;

import java.util.List;

public final class CpuComServiceLog {

    public static final class LogID {
        public static final Integer Created = 1;
        public static final Integer Destroyed = 2;
        public static final Integer NotBindableService = 3;
        public static final Integer ExceptionOnStartCommand = 4;
        public static final Integer ExceptionStartComplete = 5;
        public static final Integer PermissinDeniedSend = 6;
        public static final Integer Send = 7;
        public static final Integer InvalidCommandSend = 8;
        public static final Integer Subscribed = 9;
        public static final Integer InvalidCommandSubscribe = 10;
        public static final Integer ListSubscribing = 11;
        public static final Integer RegisteredErrorCallBack = 12;
        public static final Integer Unsubscribed = 13;
        public static final Integer UnsubscribeNotExistSubscription = 14;
        public static final Integer InvalidCommandUnsubscribe = 15;
        public static final Integer ExceptionOnRecvBytes = 16;
        public static final Integer ExceptionOnRecvErrorBytes = 17;
        public static final Integer ExceptionCallErrorCallBackByPID = 18;
        public static final Integer UnregisteredErrorCallBack = 26;
        public static final Integer ListUnsubscribing = 27;
        public static final Integer ExceptionWhileOnCommandBroadcast = 28;
        public static final Integer ExceptionWhileOnErrorBroadcast = 29;
        public static final Integer OnCommand = 31;
        public static final Integer OnError = 32;
        public static final Integer ClientConnected = 33;
        public static final Integer ClientDisconnected = 34;
        public static final Integer CanNotCreateClient = 35;
        public static final Integer ExceptionStopComplete = 36;
        public static final Integer ExceptionRegisterService = 37;
        public static final Integer OnAppStart = 38;
        public static final Integer OnAppRestart = 39;
        public static final Integer OnAppStop = 40;
        public static final Integer OnAppResume = 41;
        public static final Integer OnStartCommand = 42;
        public static final Integer ExceptionResumeComplete = 43;
        public static final Integer ExceptionRestartComplete = 44;
        public static final Integer PermissinDeniedSubscribe = 45;
    }

    public static final List<LogFormat> logMessageFormats =
            asList(
                    new LogFormat(LogID.Created, "Created\n"),
                    new LogFormat(LogID.Destroyed, "Destroyed\n"),
                    new LogFormat(LogID.NotBindableService, "Service not bindable\n"),
                    new LogFormat(
                            LogID.ExceptionOnStartCommand, "onStartCommand: Runtime exception.\n"),
                    new LogFormat(
                            LogID.ExceptionStartComplete,
                            "VehiclePowerServiceManager.notifyStartComplete: Remote exception.\n"),
                    new LogFormat(
                            LogID.ExceptionStopComplete,
                            "VehiclePowerServiceManager.stopProcessingComplete: Remote exception.\n"),
                    new LogFormat(
                            LogID.ExceptionResumeComplete,
                            "VehiclePowerServiceManager.resumeComplete: Remote exception.\n"),
                    new LogFormat(
                            LogID.ExceptionRestartComplete,
                            "VehiclePowerServiceManager.restartComplete: Remote exception.\n"),
                    new LogFormat(
                            LogID.ExceptionRegisterService, "registerService: Remote exception.\n"),
                    new LogFormat(
                            LogID.PermissinDeniedSend,
                            "[%02x,%02x] has not been sent. Reason: Permission denied. Caller pid: %d. "
                                    + "Process name: '%s'\n",
                            asList(
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"),
                                    new DisplayTypeDecInt("Caller pid"),
                                    new DisplayTypeString(1024, "Process name"))),
                    new LogFormat(
                            LogID.Send,
                            "Caller pid: %d has sent [%02x,%02x].\n",
                            asList(
                                    new DisplayTypeDecInt("Caller pid"),
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"))),
                    new LogFormat(
                            LogID.InvalidCommandSend,
                            "[%02x,%02x] has not been sent. Reason: command is invalid. Caller pid: %d.\n",
                            asList(
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"),
                                    new DisplayTypeDecInt("Caller pid"))),
                    new LogFormat(
                            LogID.Subscribed,
                            "Caller pid: %d has been subscribed to [%02x,%02x].\n",
                            asList(
                                    new DisplayTypeDecInt("Caller pid"),
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"))),
                    new LogFormat(
                            LogID.InvalidCommandSubscribe,
                            "Caller pid: %d has not been subscribed to [%02x,%02x]. Reason: command is invalid.\n",
                            asList(
                                    new DisplayTypeDecInt("Caller pid"),
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"))),
                    new LogFormat(
                            LogID.ListSubscribing,
                            "Caller pid: %d are subscribing to list of %d commands.\n",
                            asList(
                                    new DisplayTypeDecInt("Caller pid"),
                                    new DisplayTypeDecInt("Command count"))),
                    new LogFormat(
                            LogID.RegisteredErrorCallBack,
                            "Caller pid: %d has registered error callback.\n",
                            asList(new DisplayTypeDecInt("Caller pid"))),
                    new LogFormat(
                            LogID.Unsubscribed,
                            "Caller pid: %d has been unsubscribed from [%02x,%02x].\n",
                            asList(
                                    new DisplayTypeDecInt("Caller pid"),
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"))),
                    new LogFormat(
                            LogID.UnsubscribeNotExistSubscription,
                            "Caller pid: %d has not been unsubscribed from [%02x,%02x]. Reason: Subscription does not exist.\n",
                            asList(
                                    new DisplayTypeDecInt("Caller pid"),
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"))),
                    new LogFormat(
                            LogID.InvalidCommandUnsubscribe,
                            "Caller pid: %d has not been unsubscribed from [%02x,%02x]. Reason: command is invalid.\n",
                            asList(
                                    new DisplayTypeDecInt("Caller pid"),
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"))),
                    new LogFormat(LogID.ExceptionOnRecvBytes, "onRecvBytes: Remote exception.\n"),
                    new LogFormat(
                            LogID.ExceptionOnRecvErrorBytes,
                            "onRecvErrorBytes: Remote exception.\n"),
                    new LogFormat(
                            LogID.ExceptionCallErrorCallBackByPID,
                            "callErrorCallBackByPID: Remote exception.\n"),
                    new LogFormat(
                            LogID.UnregisteredErrorCallBack,
                            "Caller pid: %d has unregistered error callback.\n",
                            asList(new DisplayTypeDecInt("Caller pid"))),
                    new LogFormat(
                            LogID.ListUnsubscribing,
                            "Caller pid: %d are unsubscribing to list of %d commands.\n",
                            asList(
                                    new DisplayTypeDecInt("Caller pid"),
                                    new DisplayTypeDecInt("Command count"))),
                    new LogFormat(
                            LogID.ExceptionWhileOnCommandBroadcast,
                            "Remote Exception while broadcasting [%02x,%02x] command to caller pid: %d\n",
                            asList(
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"),
                                    new DisplayTypeDecInt("Caller pid"))),
                    new LogFormat(
                            LogID.ExceptionWhileOnErrorBroadcast,
                            "Remote Exception while broadcasting [%02x,%02x] error to caller pid: %d\n",
                            asList(
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"),
                                    new DisplayTypeDecInt("Caller pid"))),
                    new LogFormat(
                            LogID.OnCommand,
                            "Broadcasting [%02x,%02x] command to caller pid: %d\n",
                            asList(
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"),
                                    new DisplayTypeDecInt("Caller pid"))),
                    new LogFormat(
                            LogID.OnError,
                            "Broadcasting [%02x,%02x] error to caller pid: %d\n",
                            asList(
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"),
                                    new DisplayTypeDecInt("Caller pid"))),
                    new LogFormat(
                            LogID.ClientConnected,
                            "+Client connected - caller pid: %d\n",
                            asList(new DisplayTypeDecInt("Caller pid"))),
                    new LogFormat(
                            LogID.ClientDisconnected,
                            "-Client disconnected - caller pid: %d\n",
                            asList(new DisplayTypeDecInt("Caller pid"))),
                    new LogFormat(
                            LogID.CanNotCreateClient,
                            "Can not create client object for caller pid: %d\n",
                            asList(new DisplayTypeDecInt("Caller pid"))),
                    new LogFormat(LogID.OnAppStart, "onAppStart\n"),
                    new LogFormat(LogID.OnAppRestart, "onAppRestart\n"),
                    new LogFormat(LogID.OnAppStop, "onAppStop\n"),
                    new LogFormat(LogID.OnAppResume, "onAppResume\n"),
                    new LogFormat(LogID.OnStartCommand, "onStartCommand\n"),
                    new LogFormat(
                            LogID.PermissinDeniedSubscribe,
                            "Subscribe for [%02x,%02x] failed. Reason: Permission denied. Caller pid: %d."
                                    + "Process name: '%s'\n",
                            asList(
                                    new DisplayTypeHexInt("Command"),
                                    new DisplayTypeHexInt("Subcommand"),
                                    new DisplayTypeDecInt("Caller pid"),
                                    new DisplayTypeString(1024, "Process name"))));
}
