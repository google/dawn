package androidx.webgpu

import android.content.Context

/**
 * Provides a [Storage] implementation that uses [G3TestStorage]. This is used for tests that run in
 * a google3 environment.
 */
object StorageProvider {
    fun get(context: Context): Storage {
        return G3TestStorage(context)
    }
}
