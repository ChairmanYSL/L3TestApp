package com.example.emvl3app;

public interface EmvCallbackHandler {
    // 比较KernelID回调
    int onKernelIdCompare(byte transType, byte[] aid, byte aidLen, byte asi, byte[] kernelId, byte kernelIdLen);

    // 当需要断电RF卡时触发
    void onRfCardPowerOff();

    // 当需要移除卡片时触发
    void onDisplayRemoveCard();

    // 当用户界面请求数据时触发，传入步骤编号
    void onUserInterfaceRequest(int step);

    // 当输出参数集需要设置时触发，传入步骤编号
    void onOutParamSet(int step);

    // 当需要显示数据时触发
    void onDisplayData();

    // 当应用数据记录结束时触发
    void onEndApplicationDataRecord();

    // 当检测到内核ID匹配时触发
    void onCompareKernelID(boolean isMatch);

    // 当交易需要在线处理时触发
    void onRequestOnlineProcessing();

    // 当交易需要用户输入PIN时触发
    void onRequestPinEntry();

    // 当终端需要最终确认交易结果时触发
    void onFinalSelectConfirm(boolean isConfirmed);


}
