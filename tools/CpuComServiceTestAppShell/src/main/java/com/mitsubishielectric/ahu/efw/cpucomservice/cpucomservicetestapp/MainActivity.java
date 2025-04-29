package com.mitsubishielectric.ahu.efw.cpucomservice.cpucomservicetestapp;

import android.support.v7.app.AppCompatActivity;
import android.content.Intent;
import android.os.Bundle;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        startService(new Intent(this, CpuComServiceTestAppShell.class));
    }

}
