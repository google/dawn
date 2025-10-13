package androidx.webgpu.helper

import android.os.Handler
import android.os.Looper
import android.view.Surface
import androidx.webgpu.Adapter
import androidx.webgpu.BackendType
import androidx.webgpu.CallbackMode.Companion.AllowSpontaneous
import androidx.webgpu.Device
import androidx.webgpu.DeviceDescriptor
import androidx.webgpu.DeviceLostCallback
import androidx.webgpu.DeviceLostCallbackInfo
import androidx.webgpu.DeviceLostReason
import androidx.webgpu.ErrorType
import androidx.webgpu.FeatureName
import androidx.webgpu.Instance
import androidx.webgpu.InstanceDescriptor
import androidx.webgpu.RequestAdapterOptions
import androidx.webgpu.RequestAdapterStatus
import androidx.webgpu.RequestDeviceStatus
import androidx.webgpu.SurfaceDescriptor
import androidx.webgpu.SurfaceSourceAndroidNativeWindow
import androidx.webgpu.UncapturedErrorCallbackInfo
import androidx.webgpu.createInstance
import androidx.webgpu.helper.Util.windowFromSurface
import androidx.webgpu.requestAdapter
import androidx.webgpu.requestDevice

public class DeviceLostException(
    public val device: Device, @DeviceLostReason public val reason: Int, message: String
) : Exception(message)

public class UncapturedErrorException(
    public val device: Device, @ErrorType public val type: Int, message: String
) : Exception(message)

private const val POLLING_DELAY_MS = 100L

public abstract class WebGpu : AutoCloseable {
    public abstract val instance: Instance
    public abstract val webgpuSurface: androidx.webgpu.Surface
    public abstract val device: Device
}

public suspend fun createWebGpu(
    surface: Surface? = null,
    instanceDescriptor: InstanceDescriptor = InstanceDescriptor(),
    requestAdapterOptions: RequestAdapterOptions = RequestAdapterOptions(),
    @FeatureName requiredFeatures: IntArray = intArrayOf()
): WebGpu {
    initLibrary()

    val instance = createInstance(instanceDescriptor)
    val webgpuSurface =
        surface?.let {
            instance.createSurface(
                SurfaceDescriptor(
                    surfaceSourceAndroidNativeWindow =
                        SurfaceSourceAndroidNativeWindow(windowFromSurface(it))
                )
            )
        }

    val adapter = requestAdapter(instance, requestAdapterOptions)
    val device = requestDevice(adapter, requiredFeatures)

    var isClosing = false
    // Long-running event poller for async methods. Can be removed when
    // https://issues.chromium.org/issues/323983633 is fixed.
    val handler = Handler(Looper.getMainLooper())
    fun nextProcess() {
        handler.postDelayed({
            if (isClosing) {
                return@postDelayed
            }
            instance.processEvents()
            nextProcess()
        }, POLLING_DELAY_MS)
    }
    nextProcess()

    return object : WebGpu() {
        override val instance = instance
        override val webgpuSurface
            get() = checkNotNull(webgpuSurface)
        override val device = device

        override fun close() {
            isClosing = true
            //device.close() // TODO(b/428866400): Uncomment when fixed.
            webgpuSurface?.close()
            instance.close()
            adapter.close()
        }
    }
}

private suspend fun requestAdapter(
    instance: Instance,
    options: RequestAdapterOptions = RequestAdapterOptions(backendType = BackendType.Vulkan),
): Adapter {
    val result = instance.requestAdapter(options)
    check(result.status == RequestAdapterStatus.Success && result.adapter != null) {
        result. message.ifEmpty { "Error requesting the adapter: $result.status" }
    }
    return result.adapter
}

private suspend fun requestDevice(adapter: Adapter, @FeatureName requiredFeatures: IntArray): Device {
    val result =
        adapter.requestDevice(
            DeviceDescriptor(
                requiredFeatures = requiredFeatures,
                deviceLostCallback = { device, reason, message ->
                                throw DeviceLostException(device, reason, message)
                            },
                uncapturedErrorCallback = { device, type, message ->
                        throw UncapturedErrorException(device, type, message)
                    },
            )
        )
    check(result.status == RequestDeviceStatus.Success && result.device != null) {
        result.message.ifEmpty { "Error requesting the device: $result.status" }
    }
    return result.device
}

/** Initializes the native library. This method should be called before making and WebGPU calls. */
public fun initLibrary() {
    System.loadLibrary("webgpu_c_bundled")
}
