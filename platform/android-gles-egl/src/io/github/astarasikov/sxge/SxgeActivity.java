package io.github.astarasikov.sxge;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

public class SxgeActivity extends Activity {
    SxgeView mView;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new SxgeView(getApplication());
		setContentView(mView);
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }
}
