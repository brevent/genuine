import androidx.annotation.Keep;

@Keep
public class nop {

    private static void nop() {
        android.util.Log.w("nop", "nop");
    }

    private static void oop() {
        android.util.Log.i("nop", "oop");
    }

}
