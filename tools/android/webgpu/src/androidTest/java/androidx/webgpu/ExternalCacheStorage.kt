package androidx.webgpu

import android.content.Context
import android.graphics.Bitmap
import java.io.File
import java.io.FileOutputStream

/**
 * A [Storage] implementation that saves bitmaps to the external cache directory. This is used for
 * tests that do not run in a google3 environment, where [TestStorage] is not available.
 */
class ExternalCacheStorage(private val context: Context) : Storage {
    override fun writeImage(imageName: String, bitmap: Bitmap) {
        val outputDir = context.externalCacheDir ?: throw Exception("externalCacheDir is null")
        if (!outputDir.exists()) {
            outputDir.mkdirs()
        }

        val outputFile = File(outputDir, imageName)
        FileOutputStream(outputFile).use { stream ->
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream)
        }
    }
}
