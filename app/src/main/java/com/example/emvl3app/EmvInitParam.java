package com.example.emvl3app;

import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Supplier;

public class EmvInitParam {
    public EmvCallbackHandler callbackHandler;
    public SdkPaypassAppexAidStruct sdkExtAid;

    // Helper classes for handling specific callback types if necessary
    public static class AuthRequest {
        public byte[] rspCode;
        public byte[] authCode;
        public int[] authCodeLen;
        public byte[] iAuthData;
        public int[] iAuthDataLen;
        public byte[] script;
        public int[] scriptLen;
    }

    public static class KernelIdRequest {
        public byte transType;
        public byte[] aid;
        public byte aidLen;
        public byte asi;
        public byte[] kernelId;
        public byte kernelIdLen;
    }
}

