package me.piebridge;

import android.content.pm.PackageManager;
import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import me.piebridge.genuine.BuildConfig;
import me.piebridge.genuine.R;

/**
 * Created by thom on 2018/10/31.
 */
public class GenuineActivity extends AppCompatActivity {

    private static final String FRAGMENT_FAKE = "fragment-fake";

    private static final int MAGIC = 0xc91e8d1e;

    private static final byte[] APK_V2_MAGIC = {'A', 'P', 'K', ' ', 'S', 'i', 'g', ' ',
            'B', 'l', 'o', 'c', 'k', ' ', '4', '2'};

    private int magic;

    private boolean mFake;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        boolean apkSignV2 = false;
        String sourceDir = getSourceDir();
        if (sourceDir != null) {
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
                    apkSignV2 = true;
                    byte[] bytes = buffer.array();
                    for (int i = 0; i < 0x10; i++) {
                        if (APK_V2_MAGIC[i] != bytes[i]) {
                            apkSignV2 = false;
                            break;
                        }
                    }
                }
            } catch (IOException ignore) {

            }
        }
        magic = -Integer.parseInt("f86r4y", Character.MAX_RADIX);
        if (!apkSignV2 || BuildConfig.VERSION_CODE != Genuine.version()) {
            mFake = true;
            showFake(R.string.unsupported_modified ^ MAGIC);
        }
    }

    protected final boolean isFake() {
        return mFake;
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
