package androidx.webgpu

import androidx.test.filters.MediumTest
import androidx.test.filters.SmallTest
import androidx.webgpu.ValidationException
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import java.nio.ByteBuffer
import java.nio.ByteOrder
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Before
import org.junit.Test

@Suppress("UNUSED_VARIABLE")
@SmallTest
class RenderBundleEncoderTest {
  private lateinit var webGpu: WebGpu
  private lateinit var device: GPUDevice
  private lateinit var defaultColorPipeline: GPURenderPipeline
  private lateinit var shaderModule: GPUShaderModule
  private lateinit var layout: GPUPipelineLayout
  private val kColorFormat = TextureFormat.RGBA8Unorm

  @Before
  fun setup() = runBlocking {
    val gpu = createWebGpu()
    webGpu = gpu
    device = gpu.device

    shaderModule = device.createShaderModule(
      ShaderModuleDescriptor(
        shaderSourceWGSL = ShaderSourceWGSL(
          """@vertex fn vsMain() -> @builtin(position) vec4<f32> {
                        return vec4<f32>(0.0, 0.0, 0.0, 1.0);
                    }
                    @fragment fn fsMain() -> @location(0) vec4<f32> {
                        return vec4<f32>(1.0, 0.0, 0.0, 1.0);
                    }
                 """.trimIndent()
        )
      )
    )

    layout = device.createPipelineLayout(PipelineLayoutDescriptor())

    defaultColorPipeline = device.createRenderPipeline(
      RenderPipelineDescriptor(
        layout = layout,
        vertex = VertexState(module = shaderModule, entryPoint = "vsMain"),
        fragment = FragmentState(
          module = shaderModule,
          entryPoint = "fsMain",
          targets = arrayOf(
            ColorTargetState(format = kColorFormat)
          )
        ),
        primitive = PrimitiveState(topology = PrimitiveTopology.TriangleList),
      )
    )
  }

  @After
  fun teardown() {
    defaultColorPipeline.close()
    shaderModule.close()
    layout.close()

    runCatching { device.destroy() }
    webGpu.close()
  }

  companion object {
    private const val BIND_GROUP_SHADER_CODE = """
            @group(0) @binding(0) var<uniform> u : f32;
            @vertex fn vs() -> @builtin(position) vec4f { return vec4f(0,0,0,1); }
            @fragment fn fs() -> @location(0) vec4f { return vec4f(u,0,0,1); }
        """
  }

  /** Helper to create a GPURenderBundleEncoder with default color format. */
  private fun createDefaultBundleEncoder(): GPURenderBundleEncoder {
    return device.createRenderBundleEncoder(
      RenderBundleEncoderDescriptor(
        colorFormats = intArrayOf(kColorFormat)
      )
    )
  }

  /** Helper to create an index buffer with padding. */
  private fun createIndexBuffer(indices: ShortArray): GPUBuffer {
    val dataSize = (indices.size * Short.SIZE_BYTES).toLong()
    val paddedSize = (dataSize + 3) and -4L
    val buffer = device.createBuffer(
      BufferDescriptor(
        size = paddedSize,
        usage = BufferUsage.Index or BufferUsage.CopyDst
      )
    )
    val data = ByteBuffer.allocateDirect(paddedSize.toInt()).order(ByteOrder.nativeOrder())
    data.asShortBuffer().put(indices)
    device.queue.writeBuffer(buffer, 0, data)
    return buffer
  }

  /** Helper to create an indirect buffer. */
  private fun createIndirectBuffer(data: IntArray): GPUBuffer {
    val byteBuffer = ByteBuffer.allocateDirect(data.size * Int.SIZE_BYTES)
      .order(ByteOrder.nativeOrder())
    byteBuffer.asIntBuffer().put(data)
    val buffer = device.createBuffer(
      BufferDescriptor(
        size = byteBuffer.capacity().toLong(),
        usage = BufferUsage.Indirect or BufferUsage.CopyDst
      )
    )
    device.queue.writeBuffer(buffer, 0, byteBuffer)
    return buffer
  }

  @Test
  fun testInsertDebugMarker() {
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.insertDebugMarker("Marker Inside Bundle")
    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }
    assertEquals(ErrorType.NoError, errorScope.type)
  }

  @Test
  fun testPopDebugGroupWithoutPushFails() {
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.popDebugGroup()  // Invalid call.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()  // Deferred error caught here.
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
  }

  @Test
  fun testPushAndPopDebugGroupSucceeds() {
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.pushDebugGroup("BundleGroup")
    bundleEncoder.popDebugGroup()  // Valid pair.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()  // Should succeed.
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
  }

  @Test
  fun testDrawWithoutPipelineFails() {
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.draw(3)  // Invalid: pipeline not set.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
  }

  @Test
  fun testDrawWithPipelineSucceeds() {
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.setPipeline(defaultColorPipeline)  // Valid.
    bundleEncoder.draw(3)  // Valid.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
  }

  @Test
  fun testSetVertexBufferInvalidUsageFails() {
    val invalidBuffer = device.createBuffer(
      BufferDescriptor(size = 16, usage = BufferUsage.CopyDst)
    )
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.setVertexBuffer(0, invalidBuffer)  // Invalid.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
    invalidBuffer.destroy()
  }

  @Test
  fun testSetVertexBufferValidSucceeds() {
    val validBuffer = device.createBuffer(
      BufferDescriptor(size = 16, usage = BufferUsage.Vertex)
    )
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.setVertexBuffer(0, validBuffer)  // Valid.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
    validBuffer.destroy()
  }

  @Test
  fun testDrawIndexedWithoutIndexBufferFails() {
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.setPipeline(defaultColorPipeline)
    bundleEncoder.drawIndexed(3)  // Invalid: index buffer not set.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
  }

  @Test
  fun testDrawIndexedValidSucceeds() {
    val indexBuffer = createIndexBuffer(shortArrayOf(0, 1, 2))
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.setPipeline(defaultColorPipeline)
    bundleEncoder.setIndexBuffer(indexBuffer, IndexFormat.Uint16)  // Valid.
    bundleEncoder.drawIndexed(3)  // Valid.

    device.pushErrorScope(ErrorFilter.Validation)
    val bundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
    indexBuffer.destroy()
    bundle.close()
  }

  @Test
  fun testDrawIndirectInvalidBufferFails() {
    val invalidBuffer = device.createBuffer(
      BufferDescriptor(size = 16, usage = BufferUsage.CopyDst)
    )
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.setPipeline(defaultColorPipeline)
    bundleEncoder.drawIndirect(invalidBuffer, 0)  // Invalid.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
    invalidBuffer.destroy()
  }

  @Test
  fun testDrawIndirectValidSucceeds() {
    val indirectBuffer = createIndirectBuffer(intArrayOf(3, 1, 0, 0))
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.setPipeline(defaultColorPipeline)
    bundleEncoder.drawIndirect(indirectBuffer, 0)  // Valid.

    device.pushErrorScope(ErrorFilter.Validation)
    val bundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
    indirectBuffer.destroy()
    bundle.close()
  }

  @Test
  fun testDrawIndexedIndirectWithoutIndexBufferFails() {
    val indirectBuffer = createIndirectBuffer(intArrayOf(3, 1, 0, 0, 0))
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.setPipeline(defaultColorPipeline)
    bundleEncoder.drawIndexedIndirect(indirectBuffer, 0)  // Invalid.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
    indirectBuffer.destroy()
  }

  @MediumTest
  @Test
  fun testDrawIndexedIndirectValidSucceeds() {
    val indirectBuffer = createIndirectBuffer(intArrayOf(3, 1, 0, 0, 0))
    val indexBuffer = createIndexBuffer(shortArrayOf(0, 1, 2))
    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.setPipeline(defaultColorPipeline)
    bundleEncoder.setIndexBuffer(indexBuffer, IndexFormat.Uint16)  // Valid.
    bundleEncoder.drawIndexedIndirect(indirectBuffer, 0)  // Valid.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
    indirectBuffer.destroy()
    indexBuffer.destroy()
  }

  @Test
  fun testDrawWithBindGroupRequiredButNotSetFails() {
    // Create pipeline requiring a bind group locally
    val bgl = device.createBindGroupLayout(
      BindGroupLayoutDescriptor(
        entries = arrayOf(
          BindGroupLayoutEntry(
            0,
            ShaderStage.Fragment,
            buffer = BufferBindingLayout(BufferBindingType.Uniform)
          )
        )
      )
    )
    val layout =
      device.createPipelineLayout(PipelineLayoutDescriptor(bindGroupLayouts = arrayOf(bgl)))
    val module = device.createShaderModule(
      ShaderModuleDescriptor(
        shaderSourceWGSL = ShaderSourceWGSL(BIND_GROUP_SHADER_CODE)
      )
    )
    val bgPipeline = device.createRenderPipeline(
      RenderPipelineDescriptor(
        layout = layout,
        vertex = VertexState(module, "vs"),
        fragment = FragmentState(module, "fs", targets = arrayOf(ColorTargetState(kColorFormat))),
        primitive = PrimitiveState(topology = PrimitiveTopology.TriangleList)
      )
    )

    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.setPipeline(bgPipeline)  // Requires bind group 0.
    bundleEncoder.draw(3)  // Invalid: Bind group 0 not set.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedRenderBundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
  }

  /**
   * Verifies that recording draw commands in a RenderBundle succeeds
   * when a required BindGroup is correctly set.
   */
  @MediumTest
  @Test
  fun testDrawWithBindGroupSetSucceeds() {
    // Create the uniform buffer resource needed by the shader.
    val buffer = device.createBuffer(BufferDescriptor(size = 4, usage = BufferUsage.Uniform))

    // Define the layout for the bind group, matching the shader.
    val bgl = device.createBindGroupLayout(
      BindGroupLayoutDescriptor(
        entries = arrayOf(
          BindGroupLayoutEntry(
            0,
            ShaderStage.Fragment,
            buffer = BufferBindingLayout(BufferBindingType.Uniform)
          )
        )
      )
    )

    // Create the actual bind group, linking the buffer to binding 0 according to the layout.
    val bindGroup = device.createBindGroup(
      BindGroupDescriptor(
        layout = bgl,
        entries = arrayOf(BindGroupEntry(0, buffer))
      )
    )

    // Define the pipeline to expect the bind group layout 'bgl' at index 0.
    val layout =
      device.createPipelineLayout(PipelineLayoutDescriptor(bindGroupLayouts = arrayOf(bgl)))
    val module = device.createShaderModule(
      ShaderModuleDescriptor(
        shaderSourceWGSL = ShaderSourceWGSL(BIND_GROUP_SHADER_CODE)
      )
    )
    // Create the render pipeline, linking the shader, layout, and required state.
    val bgPipeline = device.createRenderPipeline(
      RenderPipelineDescriptor(
        layout = layout,
        vertex = VertexState(module, "vs"),
        fragment = FragmentState(module, "fs", targets = arrayOf(ColorTargetState(kColorFormat))),
        primitive = PrimitiveState(topology = PrimitiveTopology.TriangleList)
      )
    )

    val bundleEncoder = createDefaultBundleEncoder()
    bundleEncoder.setPipeline(bgPipeline)
    bundleEncoder.setBindGroup(0, bindGroup)  // Valid.
    bundleEncoder.draw(3)  // Valid.

    device.pushErrorScope(ErrorFilter.Validation)
    // Finish recording. Validation occurs here.
    val unusedRenderBundle = bundleEncoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
  }

  @Test
  fun testOperationAfterFinishFails() {
    val bundleEncoder = createDefaultBundleEncoder()
    val bundle = bundleEncoder.finish()  // Encoder is now consumed.
    bundle.close()  // Close the bundle itself.

    assertThrows(ValidationException::class.java) {
      bundleEncoder.draw(3)
    }
  }
}