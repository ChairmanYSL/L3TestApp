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
}
