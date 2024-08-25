# ·2023-04-27

1.更新内核RRP处理部分，适配易总提供的驱动库的接口，易总的驱动库接口可能由于精度太高的原因，每次APDU前后取时钟滴答都是一样的数，导致内核计算错误，现在取到时钟滴答相同时，认为APDU交互时间为1ms，更新接口:Paypass_RelayResistanceProtocol
2.更新DEK处理部分，客户不支持按键取消交易功能，在内核中将回调函数置为NULL屏蔽该功能，更新接口：Paypass_ResvDEKSignal
3.同步JCB内核修复AID和CAPK刷机后初次加载文件的bug，修改接口为：sdkEMVBaseLoadAIDList、sdkEMVBaseLoadCAPKList
4.根据L3案例调整SDK层RRP相关的初始化参数

5.在SDK层新增两个接口，用于对外提供获取内核编译时间和内核版本的功能

6.删除一些临时的调试信息

# ·2023-04-28

1.修复了一个APDU响应解析的bug，该bug导致卡片在回复非9000时，交易结果与规范预期可能会有偏差，修改接口：sdkEMVBase_DealCLCardData

# ·2023-05-19

1.修复了一个加载AID参数的bug，该bug由日本J16团队提出，在AID参数中设置9F1A无法设进内核，内核在读取加减的AID文件时没有检查9F1A，修改接口：TlvToTERMINFO

# ·2023-06-08

1.修复一个返回码错误问题，联机拒绝时应该返回脱机拒绝结果返回了脱机批准

# ·2023-06-09

1.修复一个内核初始化的bug，该bug导致内核中断使用重新开始新一笔交易时可能会崩溃
