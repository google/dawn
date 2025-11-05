package androidx.webgpu.helper

import android.os.Handler
import android.os.Looper
import android.view.Surface
import androidx.webgpu.GPUAdapter
import androidx.webgpu.BackendType
import androidx.webgpu.GPUDevice
import androidx.webgpu.DeviceDescriptor
import androidx.webgpu.DeviceLostCallback
import androidx.webgpu.DeviceLostReason
import androidx.webgpu.ErrorType
import androidx.webgpu.GPUInstance
import androidx.webgpu.InstanceDescriptor
import androidx.webgpu.RequestAdapterOptions
import androidx.webgpu.RequestAdapterStatus
import androidx.webgpu.GPUSurface
import androidx.webgpu.RequestDeviceStatus
import androidx.webgpu.SurfaceDescriptor
import androidx.webgpu.SurfaceSourceAndroidNativeWindow
import androidx.webgpu.UncapturedErrorCallback
import androidx.webgpu.createInstance
import androidx.webgpu.helper.Util.windowFromSurface
import java.util.concurrent.Executor

public class DeviceLostException(
    public val device: GPUDevice, @DeviceLostReason public val reason: Int, message: String
) : Exception(message)

public class ValidationException(public val device: GPUDevice, message: String) : Exception(message)

public class OutOfMemoryException(public val device: GPUDevice, message: String) : Exception(message)

public class InternalException(public val device: GPUDevice, message: String) : Exception(message)

public class UnknownException(public val device: GPUDevice, message: String) : Exception(message)

private const val POLLING_DELAY_MS = 100L

public abstract class WebGpu : AutoCloseable {
    public abstract val instance: GPUInstance
    public abstract val webgpuSurface: GPUSurface
    public abstract val device: GPUDevice
}

public suspend fun createWebGpu(
    surface: Surface? = null,
    instanceDescriptor: InstanceDescriptor = InstanceDescriptor(),
    requestAdapterOptions: RequestAdapterOptions = RequestAdapterOptions(),
    deviceDescriptor: DeviceDescriptor = DeviceDescriptor(
        deviceLostCallback = defaultDeviceLostCallback,
        deviceLostCallbackExecutor = Executor(Runnable::run),
        uncapturedErrorCallback = defaultUncapturedErrorCallback,
        uncapturedErrorCallbackExecutor = Executor(Runnable::run)
    ),
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
    val device = requestDevice(adapter, deviceDescriptor)

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
    instance: GPUInstance,
    options: RequestAdapterOptions = RequestAdapterOptions(backendType = BackendType.Vulkan),
): GPUAdapter {
    return instance.requestAdapter(options)
}

private suspend inline fun requestDevice(
    adapter: GPUAdapter,
    deviceDescriptor: DeviceDescriptor,
): GPUDevice {
    if (deviceDescriptor.deviceLostCallback == null) {
        deviceDescriptor.deviceLostCallback = defaultDeviceLostCallback
    }

    if (deviceDescriptor.uncapturedErrorCallback == null) {
        deviceDescriptor.uncapturedErrorCallback = defaultUncapturedErrorCallback
    }
    return adapter.requestDevice(deviceDescriptor)
}

private val defaultUncapturedErrorCallback get(): UncapturedErrorCallback {
    return UncapturedErrorCallback { device, type, message ->
        when (type) {
            ErrorType.NoError -> {} // NoError
            ErrorType.Validation -> throw ValidationException(device, message)
            ErrorType.OutOfMemory -> throw OutOfMemoryException(device, message)
            ErrorType.Internal -> throw InternalException(device, message)
            ErrorType.Unknown -> throw UnknownException(device, message)
            else -> throw UnknownException(device, message)
        }
    }
}

private val defaultDeviceLostCallback get(): DeviceLostCallback {
    return DeviceLostCallback { device, reason, message ->
        throw DeviceLostException(device, reason, message)
    }
}

/** Initializes the native library. This method should be called before making and WebGPU calls. */
public fun initLibrary() {
    System.loadLibrary("webgpu_c_bundled")
}
