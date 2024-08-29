package com.example.emvl3app;

public class EmvCandidate {
    public byte[] aid = new byte[16];
    public byte aidLen;
    public byte[] label = new byte[16];
    public byte labelLen;
    public byte[] preferredName = new byte[16];
    public byte preferredNameLen;
    public byte priority;
    public byte[] rsv = new byte[4];  // Reserved bytes

    // Constructor, getters, and setters are not shown for brevity
}

