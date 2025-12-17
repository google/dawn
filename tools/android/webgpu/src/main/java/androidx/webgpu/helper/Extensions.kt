package androidx.webgpu.helper

import android.graphics.Color
import android.os.Build
import androidx.annotation.RequiresApi
import androidx.webgpu.GPUColor

/**
 * Converts an Android color integer to a [GPUColor].
 * @return The [GPUColor] representation of the integer color.
 */
public fun Int.toGPUColor(): GPUColor {
    val a = Color.alpha(this) / 255.0
    val r = Color.red(this) / 255.0
    val g = Color.green(this) / 255.0
    val b = Color.blue(this) / 255.0

    return GPUColor(r, g, b, a)
}

/**
 * Converts an Android color long to a [GPUColor].
 * @return The [GPUColor] representation of the long color.
 */
@RequiresApi(Build.VERSION_CODES.O)
public fun Long.toGPUColor(): GPUColor {
    val r = Color.red(this).toDouble()
    val g = Color.green(this).toDouble()
    val b = Color.blue(this).toDouble()
    val a = Color.alpha(this).toDouble()

    return GPUColor(r, g, b, a)
}