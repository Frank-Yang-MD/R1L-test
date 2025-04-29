/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice.cpucomservicetestapp;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.RemoteException;
import android.util.Log;

import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuComManager;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static com.mitsubishielectric.ahu.efw.cpucomservice.cpucomservicetestapp.CpuComServiceTestAppShell.APP_TAG;

public final class ApiCallIntentReceiver extends BroadcastReceiver {

    private static final String TAG = APP_TAG + ".MainIntentReceiver";

    private static final String METHOD_ARG       = "method";
    private static final String COMMAND_ARG      = "cmd";
    private static final String COMMAND_LIST_ARG = "cmdList";
    private static final String DATA_ARG         = "data";

    private static final int COMMAND_ARG_LENGTH      = 2;
    private static final int COMMAND_ARG_CMD_POS     = 0;
    private static final int COMMAND_ARG_SUB_CMD_POS = 1;

    private static final String METHOD_sendCmd           = "sendCmd";
    private static final String METHOD_subscribeCB       = "subscribeCB";
    private static final String METHOD_unsubscribeCB     = "unsubscribeCB";
    private static final String METHOD_subscribeListCB   = "subscribeListCB";
    private static final String METHOD_unsubscribeListCB = "unsubscribeListCB";

    private String mMethodArg;
    private String[] mCommandArg;
    private String[] mCommandListArg;
    private String[] mDataArg;

    private CpuComManager mCpuComManager = CpuComManager.getInstance();
    private ICpuComServiceListener mCpuComServiceListener = new ICpuComServiceListener.Stub() {

        @Override
        public void onReceiveCmd(CpuCommand cpuCommand) throws RemoteException {
            Log.i(TAG, String.format("ICpuComServiceListener_onReceiveCmd: '%s'",
                    cpuCommand.toString()));
        }
    };

    @Override
    public void onReceive(Context context, Intent intent) {
        readArgs(intent);

        if (mMethodArg == null) {
            Log.e(TAG, "Please, add method argument. See README.txt");
            return;
        }

        switch (mMethodArg) {
            case METHOD_sendCmd:
                doSendCmd();
                break;
            case METHOD_subscribeCB:
                doSubscribeCB();
                break;
            case METHOD_unsubscribeCB:
                doUnsubscribeCB();
                break;
            case METHOD_subscribeListCB:
                doSubscribeListCB();
                break;
            case METHOD_unsubscribeListCB:
                doUnsubscribeListCB();
                break;
        }
    }

    private void readArgs(Intent intent) {
        if (intent.hasExtra(METHOD_ARG)) {
            mMethodArg = intent.getStringExtra(METHOD_ARG);
        } else {
            mMethodArg = null;
        }

        if (intent.hasExtra(COMMAND_ARG)) {
            mCommandArg = intent.getStringArrayExtra(COMMAND_ARG);
        } else {
            mCommandArg = null;
        }

        if (intent.hasExtra(COMMAND_LIST_ARG)) {
            mCommandListArg = intent.getStringArrayExtra(COMMAND_LIST_ARG);
        } else {
            mCommandListArg = null;
        }

        if (intent.hasExtra(DATA_ARG)) {
            mDataArg = intent.getStringArrayExtra(DATA_ARG);
        } else {
            mDataArg = null;
        }
    }

    private boolean validateCmdArg() {
        if ((mCommandArg == null) || (mCommandArg.length != COMMAND_ARG_LENGTH)) {
            Log.e(TAG, String.format("Invalid cpu command: %s. " +
                    "CpuCommand must contain [Cmd,SubCmd]", Arrays.toString(mCommandArg)));
            return false;
        }

        return true;
    }

    private boolean validateCmdListArg() {
        if ((mCommandListArg == null) || (mCommandListArg.length  % COMMAND_ARG_LENGTH != 0)) {
            Log.e(TAG, String.format("Invalid cpu command list: %s. " +
                    "CpuCommand list must contain multiple of 2 items. Count items: '%d'",
                    Arrays.toString(mCommandListArg), mCommandListArg.length));
            return false;
        }

        return true;
    }

    private void doSendCmd() {
        if (!validateCmdArg()) {
            return;
        }

        CpuCommand cpuCommand = getCpuCommandFromArgs(
                mCommandArg[COMMAND_ARG_CMD_POS],
                mCommandArg[COMMAND_ARG_SUB_CMD_POS], true);
        if (cpuCommand != null) {
            Log.i(TAG, String.format("Call API method '%s' with cmd - '%s', data - '%s'",
                    METHOD_sendCmd, cpuCommand.toString(), Arrays.toString(cpuCommand.data)));
            mCpuComManager.sendCmd(cpuCommand);
        } else {
            Log.e(TAG, "Failed to send cmd");
        }
    }

    private void doSubscribeCB() {
        if (!validateCmdArg()) {
            return;
        }

        CpuCommand cpuCommand = getCpuCommandFromArgs(
                mCommandArg[COMMAND_ARG_CMD_POS],
                mCommandArg[COMMAND_ARG_SUB_CMD_POS], false);
        if (cpuCommand != null) {
            Log.i(TAG, String.format("Call API method '%s' with cmd - '%s', data - '%s'",
                    METHOD_subscribeCB, cpuCommand.toString(), Arrays.toString(cpuCommand.data)));
            mCpuComManager.subscribeCB(cpuCommand, mCpuComServiceListener);
        } else {
            Log.e(TAG, "Failed to subscribe for cmd cmd");
        }
    }

    private void doUnsubscribeCB() {
        if (!validateCmdArg()) {
            return;
        }

        CpuCommand cpuCommand = getCpuCommandFromArgs(
                mCommandArg[COMMAND_ARG_CMD_POS],
                mCommandArg[COMMAND_ARG_SUB_CMD_POS], false);
        if (cpuCommand != null) {
            Log.i(TAG, String.format("Call API method '%s' with cmd - '%s', data - '%s'",
                    METHOD_unsubscribeCB, cpuCommand.toString(), Arrays.toString(cpuCommand.data)));
            mCpuComManager.unsubscribeCB(cpuCommand, mCpuComServiceListener);
        } else {
            Log.e(TAG, "Failed to unsubscribe for cmd cmd");
        }
    }

    private void doSubscribeListCB() {
        if (!validateCmdListArg()) {
            return;
        }

        List<CpuCommand> subscribeList = new ArrayList<>();

        for (int i = 0; i < mCommandListArg.length; i += COMMAND_ARG_LENGTH) {
            CpuCommand cpuCommand = getCpuCommandFromArgs(
                    mCommandListArg[i + COMMAND_ARG_CMD_POS],
                    mCommandListArg[i +     COMMAND_ARG_SUB_CMD_POS], false);
            if (cpuCommand == null) {
                Log.e(TAG, String.format("Failed parse command number: '%d'",
                        i / COMMAND_ARG_LENGTH));
                return;
            }

            subscribeList.add(cpuCommand);
        }
        Log.i(TAG, String.format("Call API method '%s'", METHOD_subscribeListCB));
        mCpuComManager.subscribeListCB(subscribeList, mCpuComServiceListener);
    }

    private void doUnsubscribeListCB() {
        if (!validateCmdListArg()) {
            return;
        }

        List<CpuCommand> subscribeList = new ArrayList<>();

        for (int i = 0; i < mCommandListArg.length; i += COMMAND_ARG_LENGTH) {
            CpuCommand cpuCommand = getCpuCommandFromArgs(
                    mCommandListArg[i + COMMAND_ARG_CMD_POS],
                    mCommandListArg[i + COMMAND_ARG_SUB_CMD_POS], false);
            if (cpuCommand == null) {
                Log.e(TAG, String.format("Failed parse command number: '%d'",
                        i / COMMAND_ARG_LENGTH));
                return;
            }

            subscribeList.add(cpuCommand);
        }
        Log.i(TAG, String.format("Call API method '%s'", METHOD_unsubscribeListCB));
        mCpuComManager.unsubscribeListCB(subscribeList, mCpuComServiceListener);
    }

    private Integer getNumberFromRawStringWith16Base(String arg) {
        String numberStr = arg.replaceAll("[^a-fA-F\\d]", "");

        if (numberStr.isEmpty()) {
            Log.e(TAG, String.format("Can't convert '%s' to number. " +
                    "Number must contain next symbols [a-f, A-F, 0-9]", arg));
            return null;
        }

        return Integer.parseInt(numberStr, 16);
    }

    private CpuCommand getCpuCommandFromArgs(String cmdStr, String subCmdStr, boolean withData) {
        Integer cmd = getNumberFromRawStringWith16Base(cmdStr);
        Integer subCmd = getNumberFromRawStringWith16Base(subCmdStr);

        if ((cmd == null) || (subCmd ==null)) {
            return null;
        }

        if ((mDataArg == null) || (!withData)) {
            return new CpuCommand(cmd, subCmd, new byte[1]);
        }

        byte[] data = new byte[mDataArg.length];

        for (int i = 0; i < mDataArg.length; ++i) {
            Integer dataItem = getNumberFromRawStringWith16Base(mDataArg[i]);
            if (dataItem == null) {return null;}

            data[i] = dataItem.byteValue();
        }

        return new CpuCommand(cmd, subCmd, data);
    }
}
