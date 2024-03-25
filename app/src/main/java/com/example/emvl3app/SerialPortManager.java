package com.example.emvl3app;

import android.content.Context;
import android.util.Log;

import com.szzt.sdk.device.Device;
import com.szzt.sdk.device.DeviceManager;
import com.szzt.sdk.device.port.SerialPort;

public class SerialPortManager {
    private static SerialPortManager instance;
    private SerialPort serialPort;
    private DeviceManager deviceManager;

    private SerialPortManager(Context context) {
        deviceManager = DeviceManager.createInstance(context);
        initSerialPort();
    }

    public static synchronized SerialPortManager getInstance(Context context) {
        if (instance == null) {
            instance = new SerialPortManager(context);
        }
        return instance;
    }

    private void initSerialPort() {
        Log.d("lishiyao", "initSerialPort: start initSerialPort");
        Device[] serial = deviceManager.getDeviceByType(Device.TYPE_SERIALPORT);
        if (serial != null && serial.length > 0) {
            serialPort = (SerialPort) serial[0];
        }
    }

    public SerialPort getSerialPort() {
        return serialPort;
    }

}

