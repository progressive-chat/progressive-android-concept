package chat.progressive.app;

import android.app.Application;
import android.content.Context;

public class ProgressiveApplication extends Application {
    static {
        System.loadLibrary("progressive_app");
    }

    private static Context appContext;

    @Override
    public void onCreate() {
        super.onCreate();
        appContext = getApplicationContext();
        nativeOnCreate();
    }

    @Override
    public void onTerminate() {
        nativeOnTerminate();
        super.onTerminate();
    }

    public static Context getContext() {
        return appContext;
    }

    private native void nativeOnCreate();
    private native void nativeOnTerminate();
}
