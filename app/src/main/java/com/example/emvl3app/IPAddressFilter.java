package com.example.emvl3app;

import android.text.InputFilter;
import android.text.Spanned;

public class IPAddressFilter implements InputFilter {
    @Override
    public CharSequence filter(CharSequence source, int start, int end, Spanned dest, int dstart, int dend) {
        if (source.equals(".")) { // 如果输入是小数点
            int dots = 0;
            // 计算已经输入的小数点数量
            for (int i = 0; i < dest.length(); i++) {
                if (dest.charAt(i) == '.') {
                    dots++;
                }
            }
            // 如果小数点数量已经达到3个，则阻止输入更多的小数点
            if (dots >= 3) {
                return "";
            }
        }
        return null; // 其他情况，不做修改，继续输入
    }
}
