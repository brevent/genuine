package me.piebridge;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import me.piebridge.genuine.BuildConfig;
import me.piebridge.genuine.R;

/**
 * Created by thom on 2018/10/31.
 */
public class GenuineActivity extends AppCompatActivity {

    private static final String TAG = "Genuine";

    private static final String FRAGMENT_FAKE = "fragment-fake";

    private static final int MAGIC = 0xc91e8d1e;

    private static final byte[] APK_V2_MAGIC = {'A', 'P', 'K', ' ', 'S', 'i', 'g', ' ',
            'B', 'l', 'o', 'c', 'k', ' ', '4', '2'};

    private static final String BINDER_PROXY = String.valueOf(new char[] {
            'a', 'n', 'd', 'r', 'o', 'i', 'd', '.', 'o', 's', '.',
            'B', 'i', 'n', 'd', 'e', 'r', 'P', 'r', 'o', 'x', 'y'
    });

    private static final String SERVICE_MANAGER = String.valueOf(new char[] {
            'a', 'n', 'd', 'r', 'o', 'i', 'd', '.', 'o', 's', '.',
            'S', 'e', 'r', 'v', 'i', 'c', 'e', 'M', 'a', 'n', 'a', 'g', 'e', 'r'
    });

    private static final String GET_SERVICE = String.valueOf(new char[] {
            'g', 'e', 't', 'S', 'e', 'r', 'v', 'i', 'c', 'e'
    });

    private int magic;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        GenuineApplication application = (GenuineApplication) getApplication();
        boolean fake = application.isFake();
        if (!fake) {
            fake = BuildConfig.VERSION_CODE != Genuine.version();
        }
        if (!fake) {
            try {
                Class<?> clazz = Class.forName(SERVICE_MANAGER);
                Method method = clazz.getMethod(GET_SERVICE, String.class);
                Object result = method.invoke(null, Context.ACTIVITY_SERVICE);
                String name = result.getClass().getName();
                if (!BINDER_PROXY.equals(name)) {
                    Log.e(TAG, name + " != " + BINDER_PROXY);
                    fake = true;
                }
            } catch (NoSuchMethodException ignore) {
                // do nothing
            } catch (IllegalAccessException ignore) {
                // do nothing
            } catch (InvocationTargetException ignore) {
                // do nothing
            } catch (ClassNotFoundException ignore) {
                // do nothing
            }
        }
        if (!fake) {
            String sourceDir = getSourceDir();
            try (
                    RandomAccessFile apk = new RandomAccessFile(sourceDir, "r")
            ) {
                ByteBuffer buffer = ByteBuffer.allocate(0x10);
                buffer.order(ByteOrder.LITTLE_ENDIAN);

                apk.seek(apk.length() - 0x6);
                apk.readFully(buffer.array(), 0x0, 0x6);
                int offset = buffer.getInt();
                if (buffer.getShort() == 0) {
                    apk.seek(offset - 0x10);
                    apk.readFully(buffer.array(), 0x0, 0x10);
                    byte[] bytes = buffer.array();
                    for (int i = 0; i < 0x10; i++) {
                        if (APK_V2_MAGIC[i] != bytes[i]) {
                            Log.e(TAG, sourceDir);
                            fake = true;
                            break;
                        }
                    }
                }
            } catch (IOException ignore) {
                // do nothing
            }
        }
        magic = -Integer.parseInt("f86r4y", Character.MAX_RADIX);
        if (fake) {
            application.setFake();
            showFake(R.string.unsupported_modified ^ MAGIC);
        }
    }

    protected final boolean isFake() {
        GenuineApplication application = (GenuineApplication) getApplication();
        return application.isFake();
    }

    private void showFake(int resId) {
        FakeFragment fragment = (FakeFragment) getSupportFragmentManager().findFragmentByTag(FRAGMENT_FAKE);
        if (fragment == null || fragment.getMessage() != resId) {
            if (fragment != null) {
                fragment.dismiss();
            }
            fragment = new FakeFragment();
            fragment.setMessage(resId ^ magic);
            fragment.show(getSupportFragmentManager(), FRAGMENT_FAKE);
        }
    }

    private String getSourceDir() {
        try {
            return getPackageManager().getApplicationInfo(getPackageName(), 0).sourceDir;
        } catch (PackageManager.NameNotFoundException ignore) {
            return null;
        }
    }

}
