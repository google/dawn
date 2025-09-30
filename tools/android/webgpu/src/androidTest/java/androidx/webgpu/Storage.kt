package androidx.webgpu

import android.graphics.Bitmap

/** Interface for components that can store test artifacts like bitmaps. */
interface Storage {

    /**
     * Write a bitmap to storage.
     *
     * @param imageName The name to give the image in storage.
     * @param bitmap The bitmap to store.
     */
    fun writeImage(imageName: String, bitmap: Bitmap)
}
