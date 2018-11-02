package me.piebridge;

import android.app.Application;

/**
 * Created by thom on 2018/10/31.
 */
public class GenuineApplication extends Application {

    static {
        System.loadLibrary("genuine");
    }

}
