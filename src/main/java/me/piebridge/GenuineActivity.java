package me.piebridge;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;
import me.piebridge.genuine.R;

/**
 * Created by thom on 2018/10/31.
 */
public class GenuineActivity extends AppCompatActivity {

    private static final String FRAGMENT_FAKE = "fragment-fake";

    private static final int MAGIC = 0xc91e8d1e;

    private int magic;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        GenuineApplication application = (GenuineApplication) getApplication();
        boolean fake = application.isFake();
        magic = -Integer.parseInt("f86r4y", Character.MAX_RADIX);
        if (fake) {
            showFake(R.string.unsupported_modified ^ MAGIC);
        }
    }

    protected final boolean isFake() {
        GenuineApplication application = (GenuineApplication) getApplication();
        return application.isFake();
    }

    private void showFake(int resId) {
        FakeFragment fragment = (FakeFragment) getSupportFragmentManager().findFragmentByTag(FRAGMENT_FAKE);
        if (fragment == null || fragment.getMessage() != (resId ^ magic)) {
            if (fragment != null) {
                fragment.dismiss();
            }
            fragment = new FakeFragment();
            fragment.setMessage(resId ^ magic);
            fragment.show(getSupportFragmentManager(), FRAGMENT_FAKE);
        }
    }

}
