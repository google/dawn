package androidx.webgpu

import android.content.Context

/**
 * Provides a [Storage] implementation that uses [ExternalCacheStorage]. This is used for tests that
 * do not run in a google3 environment.
 */
internal object StorageProvider {
    fun get(context: Context): Storage {
        return ExternalCacheStorage(context)
    }
}
