package com.example.emvl3app;

import android.app.Application;
import android.content.Intent;
import android.media.AudioFormat;
import android.media.AudioTrack;
import android.os.Handler;
import android.util.Log;
import android.widget.Toast;

import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import com.szzt.android.util.SzztDebug;
import com.szzt.sdk.device.Device;
import com.szzt.sdk.device.DeviceManager;
import com.szzt.sdk.device.DeviceManager.DeviceManagerListener;
import com.szzt.sdk.device.FileSystem;
import com.szzt.sdk.device.barcode.Barcode;
import com.szzt.sdk.device.barcode.CameraScan;
import com.szzt.sdk.device.card.ContactlessCardReader;
import com.szzt.sdk.device.card.IDCardReader;
import com.szzt.sdk.device.card.MagneticStripeCardReader;
import com.szzt.sdk.device.card.SmartCardReader;
import com.szzt.sdk.device.emv.EmvInterface;
import com.szzt.sdk.device.led.Led;
import com.szzt.sdk.device.port.SerialPort;
import com.szzt.sdk.system.HwSecurityManager;
import com.szzt.sdk.system.SystemManager;
import com.szzt.sdk.system.net.NetManager;

import java.io.IOException;
import java.io.InputStream;

public class MainApplication extends Application {
    private DeviceManager mDeviceManager;
    private SystemManager mSystemManager;
    private boolean isConnect = false;
    LocalBroadcastManager mLocalBroadcastManager;
    public static String ACTION_SERVICE_CONNECTED = "DEVICEMANAGER_SERVICE_CONNECTED";
    public static String ACTION_SERVICE_DISCONNECTED = "DEVICEMANAGER_SERVICE_DISCONNECTED";
    public SerialPortManager serialPortManager;
    public SerialPort serialPort;
    private static MainApplication instance;

    @Override
    public void onCreate() {
        super.onCreate();
        instance = this;

        mDeviceManager = DeviceManager.createInstance(this);
        mDeviceManager.start(deviceManagerListener);
        getSystemManager();
        mLocalBroadcastManager = LocalBroadcastManager.getInstance(this);
        serialPortManager = SerialPortManager.getInstance(this);
    }

    public static MainApplication getInstance(){
        return instance;
    }

    public SerialPort getSerialPortWrapperImpl() {
        Device[] serial = mDeviceManager.getDeviceByType(Device.TYPE_SERIALPORT);
        if (serial != null){
            return (SerialPort) serial[0];
        }
        return null;
    }

    public SerialPortManager getSerialPortManager(){
        return this.serialPortManager;
    }

    public Led getLedWrapperImpl() {
        Device[] led = mDeviceManager.getDeviceByType(Device.TYPE_LED);
        if (led != null)
            return (Led) led[0];
        return null;
    }

    public FileSystem getFileSystemWrapperImpl() {
        Device[] fileSystem = mDeviceManager.getDeviceByType(Device.TYPE_FILESYSTEM);
        if (fileSystem != null)
            return (FileSystem) fileSystem[0];
        return null;
    }

    public SmartCardReader getSmartCardReader() {
        Device[] smartCardReaders = mDeviceManager.getDeviceByType(Device.TYPE_SMARTCARDREADER);
        if (smartCardReaders != null)
            return (SmartCardReader) smartCardReaders[0];
        return null;
    }

    public EmvInterface getEmvInterface() {
        return mDeviceManager.getEmvInterface();
    }


    public ContactlessCardReader getContactlessCardReader() {
        Device[] contactlessCards = mDeviceManager
                .getDeviceByType(Device.TYPE_CONTACTLESSCARDREADER);
        if (contactlessCards != null)
            return (ContactlessCardReader) contactlessCards[0];
        return null;
    }

    public MagneticStripeCardReader getMagneticStripeCardReader() {
        Device[] magStripCards = mDeviceManager
                .getDeviceByType(Device.TYPE_MAGSTRIPECARDREADER);
        if (magStripCards != null)
            return (MagneticStripeCardReader) magStripCards[0];

        return null;
    }

    public IDCardReader getIDCardReader() {
        Device[] idCards = mDeviceManager.getDeviceByType(Device.TYPE_IDCARDREADER);
        if (idCards != null)
            return (IDCardReader) idCards[0];
        return null;
    }

    public SystemManager getSystemManager() {
        if (mSystemManager == null) {
            mSystemManager = SystemManager.getInstance(this, new SystemManager.SystemManagerListener() {
                @Override
                public int serviceEventNotify(int i) {
                    return 0;
                }
            });
        }
        return mSystemManager;
    }

    public HwSecurityManager getHwSecurityManager() {
        getSystemManager();
        if (mSystemManager != null) {
            return mSystemManager.getHwSecurityManager();
        } else return null;
    }

    public NetManager getNetworkManager() {
        getSystemManager();
        return mSystemManager.getNetManager();
    }

    public boolean isDeviceManagerConnetcted() {
        return isConnect;
    }

    @Override
    public void onTerminate() {
        mDeviceManager.stop();
        DeviceManager.destroy();
        mDeviceManager = null;
        super.onTerminate();
    }

    public CameraScan getCameraScanImpl() {
        Device[] barcode = mDeviceManager.getDeviceByType(Device.TYPE_CAMERA_SCAN);
        if (barcode != null)
            return (CameraScan) barcode[0];

        return null;
    }

    public Barcode getBarcodeImpl() {
        Device[] barcode = mDeviceManager.getDeviceByType(Device.TYPE_BARCODE);
        if (barcode != null)
            return (Barcode) barcode[0];

        return null;
    }

    DeviceManagerListener deviceManagerListener = new DeviceManagerListener() {
        @Override
        public int serviceEventNotify(
                int event) {
            if (event == EVENT_SERVICE_CONNECTED) {
                isConnect = true;
                //mLocalBroadcastManager.sendBroadcast(mServiceConnectedIntent);
            } else if (event == EVENT_SERVICE_VERSION_NOT_COMPATABLE) {
                Toast.makeText(MainApplication.this, "SDK Version is not compatable!!!", Toast.LENGTH_SHORT).show();
            } else if (event == EVENT_SERVICE_DISCONNECTED) {
                isConnect = false;
                mDeviceManager.start(deviceManagerListener);
                //mLocalBroadcastManager.sendBroadcast(mServiceDisConnectedIntent);
            }
            return 0;
        }

        @Override
        public int deviceEventNotify(
                Device device,
                int event) {
            // TODO Auto-generated method stub
            return 0;
        }
    };

}
