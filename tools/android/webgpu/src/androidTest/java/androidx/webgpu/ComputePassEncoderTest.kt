package androidx.webgpu

import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import java.nio.ByteBuffer
import java.nio.ByteOrder
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test

@Suppress("UNUSED_VARIABLE")
class ComputePassEncoderTest {
  private var webGpu: WebGpu? = null
  private lateinit var device: GPUDevice
  private lateinit var pipeline: GPUComputePipeline

  @Before
  fun setup() = runBlocking {
    val gpu = createWebGpu()
    webGpu = gpu
    device = gpu.device

    // Create a minimal compute pipeline to be used by all tests
    val shaderModule = device.createShaderModule(
      ShaderModuleDescriptor(
        shaderSourceWGSL = ShaderSourceWGSL(
          """
                    @compute @workgroup_size(1) fn main() {}
                    """.trimIndent()
        )
      )
    )

    val layout = device.createPipelineLayout(PipelineLayoutDescriptor())

    pipeline = device.createComputePipeline(
      ComputePipelineDescriptor(
        layout = layout,
        compute = ComputeState(module = shaderModule, entryPoint = "main")
      )
    )
  }

  @After
  fun teardown() {
    pipeline.close()
    runCatching { device.destroy() }
    webGpu?.close()
    webGpu = null
  }

  /**
   * Converts an IntArray into a direct ByteBuffer.
   */
  private fun createIntBuffer(data: IntArray): ByteBuffer {
    val byteBuffer = ByteBuffer.allocateDirect(data.size * Int.SIZE_BYTES)
      .order(ByteOrder.nativeOrder())
    byteBuffer.asIntBuffer().put(data)
    return byteBuffer
  }


  /**
   * Verifies that `insertDebugMarker` can be called without error.
   * This is a "smoke test".
   */
  @Test
  fun testInsertDebugMarker() {
    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginComputePass()
    device.pushErrorScope(ErrorFilter.Validation)
    passEncoder.insertDebugMarker("My Marker")

    passEncoder.end()
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }
    assertEquals(ErrorType.NoError, errorScope.type)
  }

  /**
   * Tests that calling `popDebugGroup` without a matching `pushDebugGroup`
   * results in a validation error.
   */
  @Test
  fun testPopDebugGroupWithoutPushFails() {
    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginComputePass()
    passEncoder.popDebugGroup() // Invalid call
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
  }

  /**
   * Tests that a balanced call to `pushDebugGroup` and `popDebugGroup`
   * does NOT result in an error.
   */
  @Test
  fun testPushAndPopDebugGroupSucceeds() {
    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginComputePass()
    passEncoder.pushDebugGroup("MyDebugGroup")  // Valid push.
    passEncoder.popDebugGroup()  // Valid pop.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
  }

  /**
   * Verifies that a valid `dispatchWorkgroups` call with a bound
   * pipeline does not produce a validation error.
   */
  @Test
  fun testDispatchWorkgroupsValid() {
    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginComputePass()

    passEncoder.setPipeline(pipeline)  // Set the valid pipeline.
    passEncoder.dispatchWorkgroups(1, 1, 1)  // Valid dispatch.

    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
  }

  /**
   * Tests that calling `dispatchWorkgroupsIndirect` with a buffer lacking
   * `BufferUsage.Indirect` results in a validation error.
   */
  @Test
  fun testDispatchWorkgroupsIndirectWithInvalidBuffer() {
    val invalidBuffer = device.createBuffer(
      BufferDescriptor(
        size = 12, // 3 * Int
        usage = BufferUsage.CopyDst  // Note: Missing BufferUsage.Indirect.
      )
    )

    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginComputePass()
    passEncoder.setPipeline(pipeline)
    passEncoder.dispatchWorkgroupsIndirect(invalidBuffer, 0)
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
    invalidBuffer.destroy()
  }

  /**
   * Tests that calling `dispatchWorkgroupsIndirect` with a valid buffer
   * (one with `BufferUsage.Indirect`) does NOT result in an error.
   */
  @Test
  fun testDispatchWorkgroupsIndirectWithValidBuffer() {
    val validBuffer = device.createBuffer(
      BufferDescriptor(
        size = 12,  // 3 * Int for X, Y, Z counts.
        usage = BufferUsage.Indirect or BufferUsage.CopyDst
      )
    )
    val dispatchData = createIntBuffer(intArrayOf(1, 1, 1))
    device.queue.writeBuffer(validBuffer, 0, dispatchData)

    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginComputePass()
    passEncoder.setPipeline(pipeline)
    passEncoder.dispatchWorkgroupsIndirect(validBuffer, 0)
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
    validBuffer.destroy()
  }

  /**
   * Tests that calling a command (e.g., `dispatchWorkgroups`) *after*
   * `end()` has been called results in a validation error.
   */
  @Test
  fun testDispatchAfterEndFails() {
    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginComputePass()
    passEncoder.setPipeline(pipeline)
    passEncoder.end()  // Pass has ended.

    passEncoder.dispatchWorkgroups(1)  // Invalid call after end.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
  }

  /**
   * Tests that calling `end()` twice on the same pass encoder
   * results in a validation error.
   */
  @Test
  fun testEndCalledTwiceFails() {
    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginComputePass()

    passEncoder.end()  // First call (valid).

    device.pushErrorScope(ErrorFilter.Validation)
    passEncoder.end()  // Second call (invalid).
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
  }
}