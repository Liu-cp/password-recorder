package com.public_utils;

// isNetworkAvailable
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkCapabilities;
import android.os.Build;
import android.widget.Toast;
// showDialog
import android.app.AlertDialog;
import android.content.DialogInterface;

public class PublicUtils {
    public static void showDialog(Context context, String title, String message) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(title);
        builder.setMessage(message);
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                // 当用户点击"OK"时关闭弹窗
                dialog.dismiss();
            }
        });
        builder.show();
    }

    private static String getCapabilities(NetworkCapabilities capabilities) {
        StringBuilder caps = new StringBuilder();
        if (capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)) {
            caps.append("INTERNET ");
        }
        if (capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED)) {
            caps.append("NOT_RESTRICTED ");
        }
        if (capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED)) {
            caps.append("VALIDATED ");
        }
        if (capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_CAPTIVE_PORTAL)) {
            caps.append("CAPTIVE_PORTAL ");
        }
        if (capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_NOT_METERED)) {
            caps.append("NOT_METERED ");
        }
        return caps.toString().trim();
    }

    // 检查网络链接可用性
    public static boolean isNetworkAvailable(Context context) {
        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);

        if (connectivityManager != null) {
            NetworkCapabilities capabilities = connectivityManager.getNetworkCapabilities(connectivityManager.getActiveNetwork());
            if ( capabilities == null ) {
                Toast.makeText(context, "capabilities is null!", Toast.LENGTH_LONG).show();
                showDialog(context, "系统信息", "capabilities is null!");
            }
            else {
                Toast.makeText(context, "caps: " + getCapabilities(capabilities), Toast.LENGTH_LONG).show();
                showDialog(context, "系统信息", "caps: " + getCapabilities(capabilities));
            }

            return capabilities != null && capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET);
        }
        Toast.makeText(context, "connectivityManager is null!", Toast.LENGTH_LONG).show();
        showDialog(context, "系统信息", "connectivityManager is null!");
        return false;
    }
}
