package com.example.emvl3app;

import android.util.Log;

import java.util.HashMap;
import java.util.Map;

public class TLVObject {
    private Map<String, String> tlvDic = new HashMap<>();

    public boolean exist(String tag) {
        return tlvDic.containsKey(tag);
    }

    public String get(String tag) {
        return tlvDic.get(tag);
    }

    public void clear() {
        tlvDic.clear();
    }

    public boolean parseTlvString(String tlv) {
        if (tlv.length() % 2 != 0) {
            Log.d("TLVObject", "parseTlvString: TLV string length is not even");
            return false;
        }

        try {
            int i = 0;
            while (i < tlv.length()) {
                String tag;
                int firstByte = Integer.parseInt(tlv.substring(i, i + 2), 16);

                // Determine tag length
                if ((firstByte & 0x1F) == 0x1F) {
                    int secondByte = Integer.parseInt(tlv.substring(i + 2, i + 4), 16);
                    if ((secondByte & 0x80) == 0x80) {
                        // Tag is 3 bytes
                        if (i + 6 > tlv.length()) {
                            Log.d("TLVObject","TLV string ends after tag");
                            return false;
                        }
                        tag = tlv.substring(i, i + 6);
                        i += 6;
                    } else {
                        // Tag is 2 bytes
                        if (i + 4 > tlv.length()) {
                            Log.d("TLVObject","TLV string ends after tag");
                            return false;
                        }
                        tag = tlv.substring(i, i + 4);
                        i += 4;
                    }
                } else {
                    // Tag is 1 byte
                    tag = tlv.substring(i, i + 2);
                    i += 2;
                }

                if (i >= tlv.length()) {
                    Log.d("TLVObject","TLV string ends after tag");
                    return false;
                }

                // Determine length
                int len;
                firstByte = Integer.parseInt(tlv.substring(i, i + 2), 16);
                if ((firstByte & 0x80) == 0x80) {
                    int numberOfLengthBytes = firstByte & 0x7F;
                    len = 0;
                    for (int j = 1; j <= numberOfLengthBytes; j++) {
                        if (i + j * 2 >= tlv.length()) {
                            Log.d("TLVObject","TLV string ends after length");
                            return false;
                        }
                        len = (len << 8) + Integer.parseInt(tlv.substring(i + j * 2, i + j * 2 + 2), 16);
                    }
                    i += (numberOfLengthBytes + 1) * 2;
                } else {
                    // b8 is 0, so length is given directly
                    len = firstByte & 0x7F;
                    i += 2;
                }

                if (i + len * 2 > tlv.length()) {
                    Log.d("TLVObject","TLV string ends after value");
                    return false;
                }

                // Extract the value
                String value = tlv.substring(i, i + len * 2);

                // Save the tag-value pair
                tlvDic.put(tag, value);

                i += len * 2;
            }
            return true;
        } catch (NumberFormatException e) {
            // If there's any error in the process
            Log.d("TLVObject","Error parsing TLV string");
            return false;
        }
    }

    public boolean parse_tlvBCD(byte[] tlv, int tlv_len) {
        int i = 0;
        try {
            while (i < tlv_len) {
                String tag;
                byte firstByte = tlv[i];

                // Determine tag length
                if ((firstByte & 0x1F) == 0x1F) {
                    if ((tlv[i + 1] & 0x80) == 0x80) {
                        // Tag is 3 bytes
                        if (i + 2 >= tlv_len) return false; // Bounds check for 3 bytes tag
                        tag = String.format("%02X", firstByte) + String.format("%02X", tlv[i + 1]) + String.format("%02X", tlv[i + 2]);
                        i += 3; // Move past the 3-byte tag
                    } else {
                        // Tag is 2 bytes
                        if (i + 1 >= tlv_len) return false; // Bounds check for 2 bytes tag
                        tag = String.format("%02X", firstByte) + String.format("%02X", tlv[i + 1]);
                        i += 2; // Move past the 2-byte tag
                    }
                } else {
                    // Tag is 1 byte
                    tag = String.format("%02X", firstByte);
                    i += 1; // Move past the 1-byte tag
                }

                if (i >= tlv_len) return false; // Bounds check

                // Determine length
                int len;
                firstByte = tlv[i];
                if ((firstByte & 0x80) == 0x80) { // If b8 is 1
                    int numberOfLengthBytes = firstByte & 0x7F; // b7~b1 gives the number of bytes for length
                    len = 0;
                    for (int j = 1; j <= numberOfLengthBytes; j++) {
                        if (i + j >= tlv_len) return false; // Bounds check
                        len = (len << 8) + (tlv[i + j] & 0xFF); // Construct the length from the subsequent bytes
                    }
                    i += numberOfLengthBytes + 1; // Move past all length bytes
                } else {
                    // b8 is 0, so length is given directly
                    len = firstByte & 0x7F;
                    i += 1; // Move past the length byte
                }

                if (i + len > tlv_len) return false; // Bounds check

                // Extract the value
                byte[] valueBytes = new byte[len];
                System.arraycopy(tlv, i, valueBytes, 0, len);
                StringBuilder sb = new StringBuilder();
                for (byte b : valueBytes) {
                    sb.append(String.format("%02X", b));
                }
                String value = sb.toString();

                // Save the tag-value pair
                tlvDic.put(tag, value);

                i += len; // Move past the value to the next tag, if exists
            }
            return true;
        } catch (Exception e) {
            // If there's any error in the process
            return false;
        }
    }

    public void save_tvstr(String tag, String value){
        tlvDic.put(tag, value);
    }

    public static boolean ContainsSequence(byte [] data, byte first, byte second){
        for (int i = 0; i < data.length - 1; i++)
        {
            if (data[i] == first && data[i + 1] == second)
            {
                return true;
            }
        }
        return false;
    }

    public static byte [] GetTLVValue(byte[] data, byte first, byte second){
        for (int i = 0; i < data.length - 1; i++)
        {
            if (data[i] == first && data[i + 1] == second)
            {
                int len = data[i + 2];
                byte[] value = new byte[len];
                System.arraycopy(data, i + 3, value, 0, len);
                return value;
            }
        }
        return null;
    }


}
