/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */
package com.mitsubishielectric.ahu.efw.cpucomservice.testapp;

import android.content.Context;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.inputmethod.InputConnection;
import android.widget.Button;
import android.widget.LinearLayout;

public class CustomizedKeyboard extends LinearLayout implements View.OnClickListener {

    private Button button1, button2, button3, button4,
            button5, button6, button7, button8,
            button9, button0, buttonDelete;

    private SparseArray<String> keyValues = new SparseArray<>();
    private InputConnection inputConnection;

    public CustomizedKeyboard(Context context) {
        this(context, null, 0);
    }

    public CustomizedKeyboard(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CustomizedKeyboard(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context, attrs);
    }

    private void init(Context context, AttributeSet attrs) {
        LayoutInflater.from(context).inflate(R.layout.keyboard, this, true);
        button1 = (Button) findViewById(R.id.button_1);
        button1.setOnClickListener(this);
        button2 = (Button) findViewById(R.id.button_2);
        button2.setOnClickListener(this);
        button3 = (Button) findViewById(R.id.button_3);
        button3.setOnClickListener(this);
        button4 = (Button) findViewById(R.id.button_4);
        button4.setOnClickListener(this);
        button5 = (Button) findViewById(R.id.button_5);
        button5.setOnClickListener(this);
        button6 = (Button) findViewById(R.id.button_6);
        button6.setOnClickListener(this);
        button7 = (Button) findViewById(R.id.button_7);
        button7.setOnClickListener(this);
        button8 = (Button) findViewById(R.id.button_8);
        button8.setOnClickListener(this);
        button9 = (Button) findViewById(R.id.button_9);
        button9.setOnClickListener(this);
        button0 = (Button) findViewById(R.id.button_0);
        button0.setOnClickListener(this);
        button1 = (Button) findViewById(R.id.button_a);
        button1.setOnClickListener(this);
        button2 = (Button) findViewById(R.id.button_b);
        button2.setOnClickListener(this);
        button3 = (Button) findViewById(R.id.button_c);
        button3.setOnClickListener(this);
        button4 = (Button) findViewById(R.id.button_d);
        button4.setOnClickListener(this);
        button5 = (Button) findViewById(R.id.button_e);
        button5.setOnClickListener(this);
        button6 = (Button) findViewById(R.id.button_f);
        button6.setOnClickListener(this);
        buttonDelete = (Button) findViewById(R.id.button_delete);
        buttonDelete.setOnClickListener(this);

        keyValues.put(R.id.button_1, "1");
        keyValues.put(R.id.button_2, "2");
        keyValues.put(R.id.button_3, "3");
        keyValues.put(R.id.button_4, "4");
        keyValues.put(R.id.button_5, "5");
        keyValues.put(R.id.button_6, "6");
        keyValues.put(R.id.button_7, "7");
        keyValues.put(R.id.button_8, "8");
        keyValues.put(R.id.button_9, "9");
        keyValues.put(R.id.button_0, "0");
        keyValues.put(R.id.button_a, "A");
        keyValues.put(R.id.button_b, "B");
        keyValues.put(R.id.button_c, "C");
        keyValues.put(R.id.button_d, "D");
        keyValues.put(R.id.button_e, "E");
        keyValues.put(R.id.button_f, "F");
    }

    @Override
    public void onClick(View view) {
        if (inputConnection == null)
            return;

        if (view.getId() == R.id.button_delete) {
            CharSequence selectedText = inputConnection.getSelectedText(0);

            if (TextUtils.isEmpty(selectedText)) {
                inputConnection.deleteSurroundingText(1, 0);
            } else {
                inputConnection.commitText("", 1);
            }
        } else {
            String value = keyValues.get(view.getId());
            inputConnection.commitText(value, 1);
        }
    }

    public void setInputConnection(InputConnection ic) {
        inputConnection = ic;
    }
}