package me.piebridge;

import android.app.Application;

/**
 * Created by thom on 2018/10/31.
 */
public class GenuineApplication extends Application {

    static {
        System.loadLibrary("genuine");
    }

    private boolean mFake;

    public final void setFake() {
        mFake = true;
    }

    public final boolean isFake() {
        return mFake;
    }

}
