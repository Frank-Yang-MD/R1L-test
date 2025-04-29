/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */
package com.mitsubishielectric.ahu.efw.cpucomservice.testapp;

import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.widget.Button;
import android.widget.EditText;

import com.mitsubishielectric.ahu.efw.lib.common.Const;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuComManager;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceErrorListener;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.ICpuComServiceListener;
import com.mitsubishielectric.ahu.efw.lib.extendedservicemanager.ExtSrvManager;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity implements View.OnFocusChangeListener{
    private final String LOG_TAG = "CpuComServiceTestApp";

    private CustomizedKeyboard mKeyboard;

    private Button mButtonESM;
    private Button mButtonCPU;
    private Button mButtonSendCmd;
    private Button mButtonSubscribeCB;
    private Button mButtonUnsubscribeCB;
    private Button mButtonSubscribeErrorCB;
    private Button mButtonUnsubscribeErrorCB;
    private EditText mCmdText;
    private EditText mSubCmdText;
    private EditText mDataText;
    private InputConnection mConnectionCmdText;
    private InputConnection mConnectionSubCmdText;
    private InputConnection mConnectionDataText;

    private List<String> mLog = new ArrayList<>();
    private static int mMaxLogLines;
    private EditText mLogText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mKeyboard = (CustomizedKeyboard) findViewById(R.id.keyboard);
        mButtonESM = findViewById(R.id.buttonStartESM);
        mButtonCPU = findViewById(R.id.buttonStartCPU);
        mButtonCPU.setEnabled(false);
        mButtonSendCmd = findViewById(R.id.buttonSendCmd);
        mButtonSendCmd.setEnabled(false);
        mButtonSubscribeCB = findViewById(R.id.buttonSubscribeCB);
        mButtonSubscribeCB.setEnabled(false);
        mButtonSubscribeErrorCB = findViewById(R.id.buttonSubscribeErrorCB);
        mButtonSubscribeErrorCB.setEnabled(false);
        mButtonUnsubscribeCB = findViewById(R.id.buttonUnsubscribeCB);
        mButtonUnsubscribeCB.setEnabled(false);
        mButtonUnsubscribeErrorCB = findViewById(R.id.buttonUnsubscribeErrorCB);
        mButtonUnsubscribeErrorCB.setEnabled(false);
        mCmdText = findViewById(R.id.cmdText);
        mCmdText.setOnFocusChangeListener(this);
        mSubCmdText = findViewById(R.id.subCmdText);
        mSubCmdText.setOnFocusChangeListener(this);
        mDataText = findViewById(R.id.dataText);
        mDataText.setOnFocusChangeListener(this);
        mConnectionCmdText = mCmdText.onCreateInputConnection(new EditorInfo());
        mConnectionSubCmdText = mSubCmdText.onCreateInputConnection(new EditorInfo());
        mConnectionDataText = mDataText.onCreateInputConnection(new EditorInfo());
        mLogText = findViewById(R.id.logText);
        mMaxLogLines = mLogText.getMaxLines();

        mButtonESM.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                logToUI("connectToEsm()");
                connectToEsm();
            }
        });

        mButtonCPU.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                logToUI("connectToCpu()");
                connectToCpu();
            }
        });

        mButtonSendCmd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                logToUI("sendCmd()");
                sendCmd();
            }
        });

        mButtonSubscribeCB.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                logToUI("subscribeCB()");
                subscribeCB();
            }
        });

        mButtonUnsubscribeCB.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                logToUI("unsubscribeCB()");
                unsubscribeCB();
            }
        });

        mButtonSubscribeErrorCB.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                logToUI("subscribeErrorCB()");
                subscribeErrorCB();
            }
        });

        mButtonUnsubscribeErrorCB.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                logToUI("unsubscribeErrorCB()");
                unsubscribeErrorCB();
            }
        });

        mKeyboard.setInputConnection(mConnectionCmdText);
    }

    @Override
    public void onFocusChange(View v, boolean hasFocus) {
        if (hasFocus) {
            if (v == mCmdText) {
                mKeyboard.setInputConnection(mConnectionCmdText);
            } else if (v == mSubCmdText) {
                mKeyboard.setInputConnection(mConnectionSubCmdText);
            } else if (v == mDataText) {
                mKeyboard.setInputConnection(mConnectionDataText);
            }
            ((EditText)v).setShowSoftInputOnFocus(false);
        }
    }

    private static String byteArrayToHex(byte[] bytes) {
        final char[] hexArray = "0123456789ABCDEF".toCharArray();
        String hexString;
        if (bytes != null) {
            char[] hexChars = new char[bytes.length * 2];
            for (int j = 0; j < bytes.length; j++) {
                int v = bytes[j] & 0xFF;
                hexChars[j * 2] = hexArray[v >>> 4];
                hexChars[j * 2 + 1] = hexArray[v & 0x0F];
            }
            hexString = new String(hexChars);
        } else {
            hexString = "";
        }

        return hexString;
    }

    public int hexStringToInt(String text) {
        int data = 0;
        if ((text != null) && (!text.isEmpty())) {
            int length = text.length();
            for (int i = 0; i < length; i++) {
                data <<= 4;
                data += Character.digit(text.charAt(i), 16);
            }
        }
        return data;
    }

    private byte[] hexStringToByteArray(String text) {
        byte[] data = null;

        if ((text != null) && (!text.isEmpty())) {
            int len = text.length();
            if ((len % 2) != 0) {
                char tmp = text.charAt(len - 1);
                text = text.substring(0, len - 1);
                text += '0';
                text += tmp;
                len = text.length();
            }
            if (text.length() > 1) {
                data = new byte[len / 2];
                for (int i = 0; i < len; i += 2) {
                    data[i / 2] = (byte) ((Character.digit(text.charAt(i), 16) << 4)
                            + Character.digit(text.charAt(i + 1), 16));
                }
            } else {
                data = new byte[1];
                data[0] = (byte) Character.digit(text.charAt(0), 16);
            }
        }

        return data;
    }

    private void logToUI(final String text) {
        MainActivity.this.runOnUiThread(new Runnable() {
            public void run() {
                if (text.length() > 0) {
                    mLog.add(text);
                }
                if (mLog.size() > mMaxLogLines) {
                    mLog.remove(0);
                }
                StringBuilder log = new StringBuilder();
                for (String str : mLog) {
                    log.append(str).append("\n");
                }
                mLogText.setText(log.toString());
            }
        });
    }

    private ICpuComServiceListener mListener = new ICpuComServiceListener.Stub() {
        @Override
        public void onReceiveCmd(final CpuCommand cpuCommand) {
            logToUI("Callback OnReceive:" + cpuCommand.toString() + "; data: [" + byteArrayToHex(cpuCommand.data) + "]");
        }
    };

    private ICpuComServiceErrorListener mErrorListener = new ICpuComServiceErrorListener.Stub() {
        @Override
        public void onError(int i, CpuCommand cpuCommand) {
            logToUI("Callback onError:" + cpuCommand.toString() + "; data: [" + byteArrayToHex(cpuCommand.data) + "]");
        }
    };

    private CpuCommand getCommandFromUI() {
        CpuCommand cpuCommand = new CpuCommand();
        try {
            cpuCommand.cmd = hexStringToInt(mCmdText.getText().toString());
            cpuCommand.subCmd = hexStringToInt(mSubCmdText.getText().toString());
            String dataText = mDataText.getText().toString();
            cpuCommand.data = hexStringToByteArray(dataText);
        } catch (NumberFormatException ex) {
            Log.e(LOG_TAG, "getCommandFromUI", ex);
            return null;
        }

        return cpuCommand;
    }

    private void connectToEsm() {
        ServiceConnection serviceConnection;
        Intent intent = new Intent();
        intent.setClassName(Const.ESM_PACKAGE, Const.ESM_SERVICE);

        serviceConnection = new ServiceConnection() {
            public void onServiceConnected(ComponentName name, IBinder binder) {
                if (binder == null) {
                    Log.e(LOG_TAG, "ESM binder is null");
                }
                ExtSrvManager.setBinder(binder);
                mButtonCPU.setEnabled(true);
                mButtonESM.setEnabled(false);
            }

            public void onServiceDisconnected(ComponentName name) {
                Log.d(LOG_TAG, "MainActivity onServiceDisconnected");
            }
        };
        bindService(intent, serviceConnection, BIND_AUTO_CREATE);
    }

    private void connectToCpu() {
        IBinder mCpuBinder = ExtSrvManager.getInstance().getService(Const.CPU_COM_SERVICE);
        if (mCpuBinder != null) {
            CpuComManager.setBinder(mCpuBinder);
            mButtonCPU.setEnabled(false);
            mButtonSubscribeCB.setEnabled(true);
            mButtonSendCmd.setEnabled(true);
            mButtonUnsubscribeCB.setEnabled(true);
            mButtonSubscribeErrorCB.setEnabled(true);
            mButtonUnsubscribeErrorCB.setEnabled(true);
            logToUI("App got binder CpuComService");
        } else {
            logToUI("App did not get binder CpuComService. Please retry.");
        }
    }

    private void sendCmd() {
        CpuCommand cpuCommand = getCommandFromUI();
        if (cpuCommand != null) {
            CpuComManager.getInstance().sendCmd(cpuCommand);
        }
    }

    private void subscribeCB() {
        CpuCommand cpuCommand = getCommandFromUI();
        if (cpuCommand != null) {
            CpuComManager.getInstance().subscribeCB(cpuCommand, mListener);
        }
    }

    private void subscribeErrorCB() {
        CpuComManager.getInstance().subscribeErrorCB(mErrorListener);
    }

    private void unsubscribeCB() {
        CpuCommand cpuCommand = getCommandFromUI();
        if (cpuCommand != null) {
            CpuComManager.getInstance().unsubscribeCB(cpuCommand, mListener);
        }
    }

    private void unsubscribeErrorCB() {
        CpuComManager.getInstance().unsubscribeErrorCB(mErrorListener);
    }
}

