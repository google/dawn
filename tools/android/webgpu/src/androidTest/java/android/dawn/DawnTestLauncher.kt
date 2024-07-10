package android.dawn

import android.dawn.helper.*
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking

fun dawnTestLauncher(
    requiredFeatures: Array<FeatureName> = arrayOf(),
    callback: suspend (device: Device) -> Unit
) {
    Util  // Hack to force library initialization.

    val instance = createInstance()

    runBlocking {
        val eventProcessor =
            launch {
                while (true) {
                    delay(200)
                    instance.processEvents()
                }
            }

        val adapter =
            instance.requestAdapter().adapter ?: throw DawnException("No adapter available")

        val device = adapter.requestDevice(
            DeviceDescriptor(requiredFeatures = requiredFeatures)
        ).device ?: throw DawnException("No device available")

        device.setUncapturedErrorCallback { type, message ->
            throw DawnException(message)
        }

        callback(device)

        device.close()
        device.destroy()
        adapter.close()

        eventProcessor.cancel()
        runBlocking {
            eventProcessor.join()
        }
        instance.close()
    }
}