package android.dawn.helper

import android.view.Surface

object Util {
    init {
        System.loadLibrary("webgpu_c_bundled")
    }

    external fun windowFromSurface(surface: Surface?): Long
}