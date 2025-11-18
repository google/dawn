package androidx.webgpu

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.SmallTest
import androidx.webgpu.helper.initLibrary
import java.util.concurrent.Executor
import junit.framework.TestCase.assertEquals
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertThrows
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
@SmallTest
class AdapterTest {
  private lateinit var instance: GPUInstance
  private lateinit var adapter: GPUAdapter

  @Before
  fun setup() = runBlocking {
    initLibrary()
    instance = createInstance()
    adapter = instance.requestAdapter()
  }

  @After
  fun teardown() {
    adapter.close()
    instance.close()
  }

  /**
   * Helper map for default limit values as defined by the WebGPU spec.
   * A full implementation would have all limits.
   */
  companion object {
    private val kDefaultLimits = mapOf(
      "maxTextureDimension2D" to 8192,
      "maxBindGroups" to 4,
      "minUniformBufferOffsetAlignment" to 256
    )
  }

  @Test
  fun adapterBackendTest() {
    val adapterInfo = adapter.getInfo()
    assertEquals(
      "The backend type should be Vulkan",
      BackendType.Vulkan, adapterInfo.backendType
    )
  }

  private suspend fun requestTestDevice(
    featureToTest: IntArray = intArrayOf(),
    limits: Limits = Limits(),
  ): GPUDevice {
    val descriptor = DeviceDescriptor(
      requiredFeatures = featureToTest,
      requiredLimits = limits,
      deviceLostCallback = DeviceLostCallback { device, reason, message ->
        throw DeviceLostException(device, reason, message)
      },
      deviceLostCallbackExecutor = Executor(Runnable::run),
      uncapturedErrorCallback = UncapturedErrorCallback { _, type, message ->
        throw RuntimeException("Uncaptured error: $type, $message")
      },
      uncapturedErrorCallbackExecutor = Executor(Runnable::run),
    )
    return adapter.requestDevice(descriptor)
  }

  /**
   * Verifies that requesting a device with default parameters returns a device
   * with Core features enabled.
   */
  @Test
  fun requestDeviceWithDefaultParametersEnablesCoreFeatures() {
    val device = runBlocking { adapter.requestDevice() }
    try {
      val features = device.getFeatures().features
      assertArrayEquals(intArrayOf(FeatureName.CoreFeaturesAndLimits), features)
    } finally {
      device.destroy()
    }
  }

  /**
   * Verifies that requesting a device with default parameters returns a device
   * with the correct, specification-defined default limits.
   */
  @Test
  fun requestDeviceWithDefaultParametersReturnsDefaultLimits() {
    val device = runBlocking { adapter.requestDevice() }
    try {
      val limits = device.getLimits()
      assertEquals(
        "maxTextureDimension2D should be the default value",
        kDefaultLimits["maxTextureDimension2D"],
        limits.maxTextureDimension2D
      )
      assertEquals(
        "maxBindGroups should be the default value",
        kDefaultLimits["maxBindGroups"],
        limits.maxBindGroups
      )
    } finally {
      device.destroy()
    }
  }

  /**
   * Tests that an adapter can only grant a device once. Subsequent requests fail.
   *
   * CTS Test: requestAdapter_stale
   * @see <a href="https://github.com/gpuweb/cts/blob/main/src/webgpu/api/operation/adapter/requestAdapter.spec.ts">WebGPU CTS Test</a>
   */
  @Test
  fun requestDeviceFailsAfterSuccessfulRequest() {
    val device = runBlocking { adapter.requestDevice() }
    device.destroy()
    assertThrows(
      "Adapter should be consumed after one device request", WebGpuException::class.java
    ) {
      runBlocking {
        val secondDeviceStatus = runBlocking { adapter.requestDevice() }
      }
    }
  }

  /**
   * Tests requesting a device with a feature that the adapter supports.
   *
   * CTSTest: features,known
   * @see <a href="https://github.com/gpuweb/cts/blob/main/src/webgpu/api/operation/adapter/requestAdapter.spec.ts">WebGPU CTS Test</a>
   */
  @Test
  fun requestDeviceWithSupportedFeatureSucceeds() {
    val featureToTest = FeatureName.CoreFeaturesAndLimits
    val device = runBlocking { requestTestDevice(intArrayOf(featureToTest)) }
    val deviceFeatures = device.getFeatures()
    assert(deviceFeatures.features.contains(featureToTest)) {
      "Device should have the requested feature: $featureToTest"
    }
    runCatching {
      device.destroy()
    }
  }

  /**
   * Tests that requesting a limit better than what the adapter supports fails.
   *
   * CTSTest: 'limit,better_than_supported'
   * @see <a href="https://github.com/gpuweb/cts/blob/main/src/webgpu/api/operation/adapter/requestAdapter.spec.ts">WebGPU CTS Test</a>
   */
  @Test
  fun requestDeviceWithBetterThanSupportedLimitFails() {
    val adapterLimits = adapter.getLimits()
    val betterLimit = adapterLimits.maxBindGroups + 1
    assertThrows("Requesting a better limit should fail", DeviceLostException::class.java) {
      runBlocking {
        requestTestDevice(limits = Limits(maxBindGroups = betterLimit))
      }
    }
  }

  /**
   * Tests that requesting a worse limit than the spec default gets clamped to the default.
   *
   * CTSTest: 'limit,worse_than_default'
   * @see <a href="https://github.com/gpuweb/cts/blob/main/src/webgpu/api/operation/adapter/requestAdapter.spec.ts">WebGPU CTS Test</a>
   */
  @Test
  fun requestDeviceWithWorseThanDefaultLimitClamps() {
    runBlocking {
      val worseLimit = kDefaultLimits.getValue("maxBindGroups") - 1
      assert(worseLimit > 0) // Ensure the value is still valid, just worse.
      val device = runBlocking { requestTestDevice(limits = Limits(maxBindGroups = worseLimit)) }
      val deviceLimits = device.getLimits()
      assertEquals(
        "Device limit should be clamped to the default",
        kDefaultLimits.getValue("maxBindGroups"),
        deviceLimits.maxBindGroups
      )
      runCatching { device.destroy() }
    }
  }

  /**
   * Tests that requesting a limit with a value that is not a power of two fails.
   *
   * CTSTest: 'limit,out_of_range'
   * @see <a href="https://github.com/gpuweb/cts/blob/main/src/webgpu/api/operation/adapter/requestAdapter.spec.ts">WebGPU CTS Test</a>
   *
   */
  @Test
  fun requestDeviceWithInvalidAlignmentLimitFails() {
    // minUniformBufferOffsetAlignment must be a power of 2. 255 is not.
    val invalidAlignment = 255
    assertThrows(
      "Alignment limit not a power of 2 should fail", DeviceLostException::class.java
    ) {
      runBlocking {
        requestTestDevice(
          limits = Limits(
            minUniformBufferOffsetAlignment = invalidAlignment
          )
        )
      }
    }
  }
}
