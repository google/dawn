package androidx.webgpu

import android.content.Context

/**
 * Factory for creating [Storage] instances. It uses [StorageProvider] to get the correct [Storage]
 * implementation based on the environment.
 */
object StorageFactory {
    fun createStore(context: Context): Storage {
        return StorageProvider.get(context)
    }
}
