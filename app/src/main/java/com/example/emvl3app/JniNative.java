package com.example.emvl3app;

public class JniNative {

    // 静态实例变量
    private static JniNative instance;

    // 私有构造函数
    private JniNative() {
        loadLibPaypass();
    }

    // 公共的获取实例的方法
    public static synchronized JniNative getInstance() {
        if (instance == null) {
            instance = new JniNative();
        }
        return instance;
    }

    // 加载库
    private void loadLibPaypass() {
        System.loadLibrary("szzt_sdkpaypass");
    }
    public native byte[] emvGetVersion();
    public native int emv_initialize(EmvInitParam param);
    public native EmvTagData emvGetTagData(int tag);
    public native int emvSetTagData(int tag, byte[] value, int length);
    public native EmvCandidate[] emvGetCandidateList();
    public native int emvSetTransAmount(long amount);
    public native int emvSetOtherAmount(long amount);
    public native int emvSetTransType(byte type);
    public native byte emvGetTransType();
    public native int emvPreProcess(boolean online);
    public native int emvProcess();
    public native int emvSetOnlinePinEntered(int result, byte[] pinBuf, int pinLen);
    public native int emvSetForceOnline(boolean flag);
    public native int emvSetOnlineResult(int result, byte [] resp, int respLen);
    public native boolean emvIsNeedSignature();
    public native int emvSetTermParam(byte[] par, int len);
    public native int emvClrAidpParam();
    public native int emvAddAidParam(byte[] par, int len);
    public native int emvClrCAPKParam();
    public native int emvAddCAPKParam(byte[] par, int len);
    public native int emvClrCardBlack();
    public native int emvAddCardBlack(EmvCardBlack cardBlack);
    public native int emvAddCertBlack(EmvCertBlack certBlack);
    public native int emvClrCertBlack();

    public static class EmvTagData{
        public byte [] value;
        public int len;
        public int ret;
    }
}
