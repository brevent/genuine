package me.piebridge;

import androidx.annotation.Keep;

import java.lang.reflect.Member;

/**
 * Created by thom on 2018/10/31.
 */
@Keep
public class Genuine {

    static {
        System.loadLibrary("genuine");
    }

    private Genuine() {

    }

    private static native Object invoke(Member m, int i, Object a, Object t, Object[] as) throws Throwable;

    public static native int version();

}
