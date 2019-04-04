package me.piebridge;

import android.app.Application;

import me.piebridge.genuine.BuildConfig;

/**
 * Created by thom on 2018/10/31.
 */
public class GenuineApplication extends Application {

    static {
        System.loadLibrary(String.valueOf(new char[] {
                'g', 'e', 'n', 'u', 'i', 'n', 'e'
        }));
    }

    private boolean mFake;

    @Override
    public void onCreate() {
        super.onCreate();
        mFake = BuildConfig.VERSION_CODE != Genuine.version();
    }

    public final void setFake() {
        mFake = true;
    }

    public final boolean isFake() {
        return mFake;
    }

}
