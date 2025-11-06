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
class RenderPassEncoderTest {
  private var webGpu: WebGpu? = null
  private lateinit var device: GPUDevice
  private lateinit var defaultColorPipeline: GPURenderPipeline
  private lateinit var renderTarget: GPUTexture
  private lateinit var renderTargetDepth: GPUTexture
  private lateinit var renderTargetView: GPUTextureView
  private lateinit var renderTargetDepthView: GPUTextureView
  private lateinit var shaderModule: GPUShaderModule
  private lateinit var layout: GPUPipelineLayout
  private val kDepthFormat = TextureFormat.Depth24Plus

  @Before
  fun setup() = runBlocking {
    val gpu = createWebGpu()
    webGpu = gpu
    device = gpu.device

    renderTarget = device.createTexture(
      TextureDescriptor(
        size = Extent3D(1, 1, 1),
        format = TextureFormat.RGBA8Unorm,
        usage = TextureUsage.RenderAttachment
      )
    )
    renderTargetView = renderTarget.createView()

    renderTargetDepth = device.createTexture(
      TextureDescriptor(
        size = Extent3D(1, 1, 1),
        format = kDepthFormat,
        usage = TextureUsage.RenderAttachment
      )
    )
    renderTargetDepthView = renderTargetDepth.createView()

    shaderModule = device.createShaderModule(
      ShaderModuleDescriptor(
        shaderSourceWGSL = ShaderSourceWGSL(
          """
                        @vertex fn vsMain() -> @builtin(position) vec4<f32> {
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
            ColorTargetState(format = TextureFormat.RGBA8Unorm)
          )
        ),
        primitive = PrimitiveState(topology = PrimitiveTopology.TriangleList),
      )
    )
  }

  @After
  fun teardown() {
    defaultColorPipeline.close()
    renderTargetView.close()
    renderTarget.destroy()

    renderTargetDepthView.close()
    renderTargetDepth.destroy()

    shaderModule.close()
    layout.close()

    runCatching { device.destroy() }
    webGpu?.close()
    webGpu = null
  }

  /**
   * Helper function to begin a standard (color-only) render pass.
   */
  private fun beginDefaultRenderPass(encoder: GPUCommandEncoder): GPURenderPassEncoder {
    return encoder.beginRenderPass(
      RenderPassDescriptor(
        colorAttachments = arrayOf(
          RenderPassColorAttachment(
            view = renderTargetView,
            loadOp = LoadOp.Clear,
            storeOp = StoreOp.Store,
            clearValue = Color(0.0, 0.0, 0.0, 1.0)
          )
        )
      )
    )
  }

  /**
   * Helper to create a buffer for index data with 4-byte padding.
   */
  private fun createIndexBuffer(indices: ShortArray): GPUBuffer {
    val dataSize = (indices.size * Short.SIZE_BYTES).toLong()
    val paddedSize = (dataSize + 3) and -4L  // Aligns to 4 bytes.

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

  /**
   * Helper to create a buffer for indirect draw calls.
   */
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
  fun testDebugMarker() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)

    passEncoder.insertDebugMarker("Drawing background")

    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
  }

  @Test
  fun testPopDebugGroupWithoutPushFails() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.popDebugGroup()  // Invalid call.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
  }

  @Test
  fun testDrawWithoutPipelineFails() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.draw(3)  // Invalid: pipeline not set.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
  }

  @Test
  fun testSetVertexBufferInvalidUsageFails() {
    val invalidBuffer = device.createBuffer(
      BufferDescriptor(size = 16, usage = BufferUsage.CopyDst)
    )
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setVertexBuffer(0, invalidBuffer)  // Invalid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
    invalidBuffer.destroy()
  }

  @Test
  fun testDrawIndexedWithoutIndexBufferFails() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setPipeline(defaultColorPipeline)
    passEncoder.drawIndexed(3)  // Invalid: index buffer not set.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
  }

  @Test
  fun testDrawIndexedValidSucceeds() {
    val indexBuffer = createIndexBuffer(shortArrayOf(0, 1, 2))
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setPipeline(defaultColorPipeline)
    passEncoder.setIndexBuffer(indexBuffer, IndexFormat.Uint16)  // Valid.
    passEncoder.drawIndexed(3)  // Valid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
    indexBuffer.destroy()
  }

  @Test
  fun testDrawIndirectInvalidBufferFails() {
    val invalidBuffer = device.createBuffer(
      BufferDescriptor(size = 16, usage = BufferUsage.CopyDst)  // 4 * Int.
    )
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setPipeline(defaultColorPipeline)
    passEncoder.drawIndirect(invalidBuffer, 0)  // Invalid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
    invalidBuffer.destroy()
  }

  @Test
  fun testDrawIndexedIndirectWithoutIndexBufferFails() {
    val indirectBuffer = createIndirectBuffer(intArrayOf(3, 1, 0, 0, 0))
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setPipeline(defaultColorPipeline)
    passEncoder.drawIndexedIndirect(indirectBuffer, 0)  // Invalid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
    indirectBuffer.destroy()
  }

  @Test
  fun testDrawIndexedIndirectValidSucceeds() {
    val indirectBuffer = createIndirectBuffer(intArrayOf(3, 1, 0, 0, 0))
    val indexBuffer = createIndexBuffer(shortArrayOf(0, 1, 2))
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setPipeline(defaultColorPipeline)
    passEncoder.setIndexBuffer(indexBuffer, IndexFormat.Uint16)  // Valid.
    passEncoder.drawIndexedIndirect(indirectBuffer, 0)  // Valid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
    indirectBuffer.destroy()
    indexBuffer.destroy()
  }

  @Test
  fun testOcclusionQueryValidSucceeds() {
    // This test needs its OWN depth-enabled pipeline.
    // We can't use the class-level 'defaultColorPipeline' because it's color-only.

    val depthPipeline = device.createRenderPipeline(
      RenderPipelineDescriptor(
        layout = layout,
        vertex = VertexState(module = shaderModule, entryPoint = "vsMain"),
        fragment = FragmentState(
          module = shaderModule,
          entryPoint = "fsMain",
          targets = arrayOf(
            ColorTargetState(format = TextureFormat.RGBA8Unorm)
          )
        ),
        primitive = PrimitiveState(topology = PrimitiveTopology.TriangleList),
        depthStencil = DepthStencilState(
          format = kDepthFormat,
          depthWriteEnabled = OptionalBool.Companion.True,
          depthCompare = CompareFunction.Always
        )
      )
    )

    val querySet = device.createQuerySet(
      QuerySetDescriptor(type = QueryType.Occlusion, count = 1)
    )
    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginRenderPass(
      RenderPassDescriptor(
        colorAttachments = arrayOf(
          RenderPassColorAttachment(
            view = renderTargetView,
            loadOp = LoadOp.Clear,
            storeOp = StoreOp.Store,
            clearValue = Color(0.0, 0.0, 0.0, 1.0)
          )
        ),
        depthStencilAttachment = RenderPassDepthStencilAttachment(
          view = renderTargetDepthView,
          depthLoadOp = LoadOp.Clear,
          depthStoreOp = StoreOp.Store,
          depthClearValue = 1.0f
        ),
        occlusionQuerySet = querySet
      )
    )

    passEncoder.beginOcclusionQuery(0)
    passEncoder.setPipeline(depthPipeline)  // Use the local depth-pipeline.
    passEncoder.draw(3)
    passEncoder.endOcclusionQuery()
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)

    querySet.destroy()
    depthPipeline.close()
  }

  @Test
  fun testSetPassPropertiesSucceeds() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.setViewport(0f, 0f, 1f, 1f, 0f, 1f)
    passEncoder.setScissorRect(0, 0, 1, 1)
    passEncoder.setBlendConstant(Color(0.0, 0.0, 0.0, 0.0))
    passEncoder.setStencilReference(0)
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)
  }

  @Test
  fun testEndCalledTwiceFails() {
    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)
    passEncoder.end()  // First call (valid).

    device.pushErrorScope(ErrorFilter.Validation)
    passEncoder.end()  // Second call (invalid).
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, errorScope.type)
  }

  @Test
  fun testExecuteBundlesSucceeds() {
    val bundleEncoder = device.createRenderBundleEncoder(
      RenderBundleEncoderDescriptor(
        colorFormats = intArrayOf(TextureFormat.RGBA8Unorm)
      )
    )
    bundleEncoder.setPipeline(defaultColorPipeline)  // Use the color-only pipeline.
    bundleEncoder.draw(3)
    val bundle = bundleEncoder.finish()

    val encoder = device.createCommandEncoder()
    val passEncoder = beginDefaultRenderPass(encoder)  // Use the color-only pass.
    passEncoder.executeBundles(arrayOf(bundle))  // Valid.
    passEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val errorScope = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, errorScope.type)

    bundle.close()
  }
}