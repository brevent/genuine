package me.piebridge;

import android.app.Activity;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.KeyEvent;

import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;

/**
 * Created by thom on 2018/10/31.
 */
public class FakeFragment extends DialogFragment
        implements DialogInterface.OnKeyListener, DialogInterface.OnClickListener {

    private static final String MESSAGE = "MESSAGE";

    public FakeFragment() {
        setArguments(new Bundle());
        setStyle(STYLE_NO_TITLE, 0);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
        Bundle arguments = getArguments();
        builder.setMessage(getString(arguments.getInt(MESSAGE)));
        builder.setPositiveButton(android.R.string.ok, this);
        builder.setOnKeyListener(this);
        return builder.create();
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        finishActivity();
    }

    private void finishActivity() {
        Activity activity = getActivity();
        if (activity != null) {
            activity.finish();
            System.exit(0);
        }
    }

    @Override
    public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK && event.getAction() == KeyEvent.ACTION_DOWN) {
            finishActivity();
        }
        return false;
    }

    public void setMessage(int resId) {
        getArguments().putInt(MESSAGE, resId);
    }

    public int getMessage() {
        return getArguments().getInt(MESSAGE);
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        finishActivity();
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        // https://stackoverflow.com/a/27084544
        if (super.getDialog() == null) {
            super.setShowsDialog(false);
        }
        try {
            super.onActivityCreated(savedInstanceState);
        } catch (NullPointerException e) { // NOSONAR
            // do nothing
        }
    }

    @Override
    public void show(FragmentManager manager, String tag) {
        Fragment fragment = manager.findFragmentByTag(tag);
        if (fragment == null || !fragment.isAdded()) {
            super.show(manager, tag);
        }
    }

}
