package androidx.webgpu

import androidx.test.filters.SmallTest
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import java.util.concurrent.Executor
import junit.framework.TestCase
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assume
import org.junit.Before
import org.junit.Test

@Suppress("UNUSED_VARIABLE")
@SmallTest
class QuerySetTest {
  private lateinit var webGpu: WebGpu
  private lateinit var device: GPUDevice

  @Before
  fun setup() = runBlocking {
    val gpu = createWebGpu(
      deviceDescriptor = DeviceDescriptor(
        requiredFeatures = intArrayOf(FeatureName.TimestampQuery),
        deviceLostCallbackExecutor = Executor(Runnable::run),
        deviceLostCallback = null,
        uncapturedErrorCallbackExecutor = Executor(Runnable::run),
        uncapturedErrorCallback = null
      ),
    )  // Request timestamp feature if available.
    webGpu = gpu
    device = gpu.device
  }

  @After
  fun teardown() {
    runCatching { device.destroy() }
    webGpu.close()
  }

  companion object {
    private const val QUERY_COUNT = 2
  }


  /** Helper to create a destination buffer for resolveQuerySet */
  private fun createResolveBuffer(size: Long): GPUBuffer {
    return device.createBuffer(
      BufferDescriptor(
        size = size,
        usage = BufferUsage.QueryResolve or BufferUsage.CopySrc  // CopySrc for potential readback.
      )
    )
  }

  @Test
  fun testCreateOcclusionQuerySet() {
    val querySet = device.createQuerySet(
      QuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
    )
    TestCase.assertNotNull(querySet)
    assertEquals(QueryType.Occlusion, querySet.type)
    assertEquals(QUERY_COUNT, querySet.count)
  }

  @Test
  fun testCreateTimestampQuerySet() {
    // Timestamp query requires a specific feature.
    if (!device.hasFeature(FeatureName.TimestampQuery)) {
      Assume.assumeTrue("testCreateTimestampQuerySet: TimestampQuery feature not supported.", true)
      return  // Skip test if feature not available.
    }

    val querySet = device.createQuerySet(
      QuerySetDescriptor(type = QueryType.Timestamp, count = QUERY_COUNT)
    )
    TestCase.assertNotNull(querySet)
    assertEquals(QueryType.Timestamp, querySet.type)
    assertEquals(QUERY_COUNT, querySet.count)
    querySet.destroy()
  }

  @Test
  fun testCreateQuerySetWithNegativeCountFails() {
    // Attempting to create a QuerySet with count -1 should fail validation.
    device.pushErrorScope(ErrorFilter.Validation)
    val unusedQuerySet =
      device.createQuerySet(QuerySetDescriptor(type = QueryType.Occlusion, count = -1))
    val error = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, error)
  }

  @Test
  fun testGetCount() {
    val querySet = device.createQuerySet(
      QuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
    )

    assertEquals(QUERY_COUNT, querySet.count)
  }

  @Test
  fun testGetType() {
    val querySet = device.createQuerySet(
      QuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
    )
    assertEquals(QueryType.Occlusion, querySet.type)
    querySet.destroy()

    if (!device.hasFeature(FeatureName.TimestampQuery)) {
      Assume.assumeTrue("testGetType: TimestampQuery feature not supported.", true)
      return  // Skip test if feature not available.
    }
    val tsQuerySet = device.createQuerySet(
      QuerySetDescriptor(type = QueryType.Timestamp, count = QUERY_COUNT)
    )
    assertEquals(QueryType.Timestamp, tsQuerySet.type)
  }


  @Test
  fun testResolveQuerySetValid() {
    val querySet = device.createQuerySet(
      QuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
    )
    // Each individual query result is stored as a 64-bit unsigned integer.
    val destinationBuffer = createResolveBuffer((QUERY_COUNT * Long.SIZE_BYTES).toLong())

    val encoder = device.createCommandEncoder()
    encoder.resolveQuerySet(querySet, 0, QUERY_COUNT, destinationBuffer, 0)

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val error = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.NoError, error)

    querySet.destroy()
    destinationBuffer.destroy()
  }

  @Test
  fun testResolveQuerySetInvalidDestinationUsage() {
    val querySet = device.createQuerySet(
      QuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
    )
    // Create buffer *without* QueryResolve usage.
    val invalidBuffer = device.createBuffer(
      BufferDescriptor(size = 8, usage = BufferUsage.CopySrc)
    )

    val encoder = device.createCommandEncoder()
    encoder.resolveQuerySet(querySet, 0, QUERY_COUNT, invalidBuffer, 0)  // Invalid usage.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val error = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, error)
  }

  @Test
  fun testResolveQuerySetDestinationTooSmall() {
    val querySet = device.createQuerySet(
      QuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
    )
    // GPUBuffer only has space for 1 result (8 bytes), but we try to resolve 2.
    val smallBuffer = createResolveBuffer((1 * Long.SIZE_BYTES).toLong())

    val encoder = device.createCommandEncoder()
    encoder.resolveQuerySet(querySet, 0, QUERY_COUNT, smallBuffer, 0)  // Invalid size.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val error = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, error)
  }

  @Test
  fun testResolveQuerySetIndexOutOfBounds() {
    val querySet = device.createQuerySet(
      QuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
    )
    // Each individual query result is stored as a 64-bit unsigned integer.
    val destinationBuffer = createResolveBuffer(QUERY_COUNT * 8L)

    val encoder = device.createCommandEncoder()
    // Try to resolve starting at index 1, count 2 (goes past end).
    encoder.resolveQuerySet(querySet, 1, QUERY_COUNT, destinationBuffer, 0)  // Invalid range.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val error = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, error)
  }

  @Test
  fun testResolveQuerySetOffsetAlignment() {
    val querySet = device.createQuerySet(
      QuerySetDescriptor(type = QueryType.Occlusion, count = QUERY_COUNT)
    )

    // Each individual query result is stored as a 64-bit unsigned integer.
    val destinationBuffer = createResolveBuffer(QUERY_COUNT * 8L + 8L)

    val encoder = device.createCommandEncoder()
    // Try to resolve starting at offset 4 (invalid alignment).
    encoder.resolveQuerySet(querySet, 0, QUERY_COUNT, destinationBuffer, 4)  // Invalid offset.

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val error = runBlocking { device.popErrorScope() }

    assertEquals(ErrorType.Validation, error)
  }
}