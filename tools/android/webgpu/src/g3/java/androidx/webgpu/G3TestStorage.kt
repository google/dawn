package androidx.webgpu

import android.content.Context
import android.graphics.Bitmap
import androidx.test.services.storage.TestStorage

/**
 * A [Storage] implementation that uses [TestStorage] to save bitmaps. This is used when running
 * tests in a google3 environment.
 */
class G3TestStorage(private val context: Context) : Storage {
    override fun writeImage(imageName: String, bitmap: Bitmap) {
        TestStorage(context.contentResolver).openOutputFile(imageName).use {
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, it)
        }
    }
}
