package com.example.emvl3app;

public class JniNative {

    static {
        loadLibPaypass();
    }
    public static void loadLibPaypass(){
        System.loadLibrary("szzt_sdkpaypass");
    }

    public native int emv_get_version(byte [] ver);



    public static byte [] getKernelVersion(int type){
        JniNative loader = new JniNative();

        byte[] version = new byte[64];
        loader.emv_get_version(version);
        return version;
    }
}
