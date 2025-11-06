package androidx.webgpu

import androidx.test.filters.SmallTest
import androidx.webgpu.helper.DawnException
import androidx.webgpu.helper.ValidationException
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createBitmap
import androidx.webgpu.helper.createWebGpu
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Before
import org.junit.Test

@Suppress("UNUSED_VARIABLE")
@SmallTest
class TextureTest {
  private lateinit var webGpu: WebGpu
  private lateinit var device: GPUDevice

  @Before
  fun setup() = runBlocking {
    webGpu = createWebGpu()
    device = webGpu.device
  }

  @After
  fun teardown() {
    runCatching { device.destroy() }
    webGpu.close()
  }

  @Test
  fun testTextureProperties() {
    val testTextureDescriptor = TextureDescriptor(
      size = Extent3D(width = 32, height = 16, depthOrArrayLayers = 1),
      format = TextureFormat.RGBA8Unorm,
      usage = TextureUsage.TextureBinding or TextureUsage.CopyDst,
      mipLevelCount = 1,
      sampleCount = 1,
      dimension = TextureDimension._2D
    )
    val texture = device.createTexture(testTextureDescriptor)
    try {
      assertEquals(testTextureDescriptor.size.width, texture.width)
      assertEquals(testTextureDescriptor.size.height, texture.height)
      assertEquals(testTextureDescriptor.size.depthOrArrayLayers, texture.depthOrArrayLayers)
      assertEquals(testTextureDescriptor.format, texture.format)
      assertEquals(testTextureDescriptor.usage, texture.usage)
      assertEquals(testTextureDescriptor.mipLevelCount, texture.mipLevelCount)
      assertEquals(testTextureDescriptor.sampleCount, texture.sampleCount)
      assertEquals(testTextureDescriptor.dimension, texture.dimension)
    } finally {
      texture.destroy()
    }
  }

  /**
   * Verifies that creating a texture with an invalid combination of usage flags
   * (e.g., multisampling with storage binding) fails validation.
   */
  @Test
  fun createTexture_withInvalidUsageCombination_fails() {
    // According to the WebGPU specification, a multisampled texture (sampleCount > 1)
    // cannot have the StorageBinding usage flag. This is a guaranteed validation error.
    val invalidDescriptor = TextureDescriptor(
      size = Extent3D(width = 32, height = 16, depthOrArrayLayers = 1),
      format = TextureFormat.RGBA8Unorm,  // A standard format is fine.
      sampleCount = 4,  // Multisampled.
      usage = TextureUsage.RenderAttachment or TextureUsage.StorageBinding // Invalid combination
    )

    assertThrows(ValidationException::class.java) {
      val unusedGPUTexture = device.createTexture(invalidDescriptor)
    }
  }

  /**
   * Verifies that using a view from a destroyed texture generates an uncaptured error
   * upon submission to the queue.
   */
  @Test
  fun useOfDestroyedTextureView_firesUncapturedError() {
    val textureDescriptor = TextureDescriptor(
      size = Extent3D(width = 32, height = 16, depthOrArrayLayers = 1),
      format = TextureFormat.RGBA8Unorm,
      usage = TextureUsage.RenderAttachment
    )

    val texture = device.createTexture(textureDescriptor)
    texture.destroy()

    val invalidView = texture.createView()

    val encoder = device.createCommandEncoder()
    try {
      val passEncoder = encoder.beginRenderPass(
        RenderPassDescriptor(
          colorAttachments = arrayOf(
            RenderPassColorAttachment(
              view = invalidView,
              loadOp = LoadOp.Clear,
              storeOp = StoreOp.Store,
              clearValue = Color(r = 1.0, g = 0.0, b = 0.0, a = 1.0)
            )
          )
        )
      )
      passEncoder.end()
      val commandBuffer = encoder.finish()

      val queue = device.getQueue()
      assertThrows(ValidationException::class.java) {
        queue.submit(arrayOf(commandBuffer))
        runBlocking {
          val unusedQueueWorkDoneReturn = queue.onSubmittedWorkDone()
        }
      }
    } finally {
      encoder.close()
    }
  }

  /**
   * Verifies that creating a bitmap from a texture with an invalid width fails.
   */
  @Test
  fun createBitmap_withInvalidWidth_fails() {
    val descriptor = TextureDescriptor(
      size = Extent3D(width = 65, height = 16, depthOrArrayLayers = 1),
      format = TextureFormat.RGBA8Unorm,
      usage = TextureUsage.CopySrc
    )
    val texture = device.createTexture(descriptor)
    assertThrows(DawnException::class.java) {
      runBlocking {
        texture.createBitmap(device)
      }
    }
  }
}