package androidx.webgpu

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.SmallTest
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assume.assumeFalse
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.atomic.AtomicBoolean

@RunWith(AndroidJUnit4::class)
@SmallTest
class AsyncHelperTest {

    private lateinit var webGpu: WebGpu
    private lateinit var device: GPUDevice

    @Before
    fun setup() {
        runBlocking {
            webGpu = createWebGpu()
            device = webGpu.device
        }
    }

    @Test
    fun asyncMethodTest() {
        runBlocking {
            /* Set up a shader module to support the async call. */
            val shaderModule = device.createShaderModule(
                ShaderModuleDescriptor(shaderSourceWGSL = ShaderSourceWGSL(""))
            )

            /* Call an asynchronous method, converted from a callback pattern by a helper. */
            val result = device.createRenderPipelineAsync(
                RenderPipelineDescriptor(vertex = VertexState(module = shaderModule))
            )

            assert(result.status == CreatePipelineAsyncStatus.ValidationError) {
                """Create render pipeline (async) should fail when no shader entry point exists.
                   The result was: ${result.status}"""
            }
        }
    }

    @Test
    fun asyncMethodTestValidationPasses() {
        runBlocking {
            /* Set up a shader module to support the async call. */
            val shaderModule = device.createShaderModule(
                ShaderModuleDescriptor(
                    shaderSourceWGSL = ShaderSourceWGSL(
                        """
@vertex fn vertexMain(@builtin(vertex_index) i : u32) ->
@builtin(position) vec4f {
    return vec4f();
}
@fragment fn fragmentMain() -> @location(0) vec4f {
    return vec4f();
}
                        """
                    )
                )
            )

            /* Call an asynchronous method, converted from a callback pattern by a helper. */
            val result = device.createRenderPipelineAsync(
                RenderPipelineDescriptor(
                    vertex = VertexState(module = shaderModule),
                    fragment = FragmentState(
                        module = shaderModule,
                        targets = arrayOf(ColorTargetState(format = TextureFormat.RGBA8Unorm))
                    )
                )
            )

          /* Create render pipeline (async) should pass with a simple shader.. */
          assertEquals(result.status, CreatePipelineAsyncStatus.Success)

        }
    }

    private fun baseCancellationTest(doCancel: Boolean): Boolean {
        val hasReturned = AtomicBoolean(false)

        runBlocking {
            val shaderModule = device.createShaderModule(
                ShaderModuleDescriptor(shaderSourceWGSL = ShaderSourceWGSL(""))
            )

            /* Launch the function in a new coroutine, giving us a job handle we can cancel. */
            val job = launch {
                device.createRenderPipelineAsync(
                    RenderPipelineDescriptor(vertex = VertexState(module = shaderModule))
                )
                hasReturned.set(true)
            }
            assumeFalse("The job completed before we could test it", hasReturned.get())

            if (doCancel) {
                job.cancel()
            }
            job.join()
        }
        return hasReturned.get()
    }

    /**
     * Test that the async-based job will complete if it's not cancelled.
     */
    @Test
    fun asyncMethodCancellationTestControl() {
        assertEquals(
            "The async job should have completed but it failed to do so.",
            true,
            baseCancellationTest(false)
        )
    }

    /**
     * Test that the async-based job will not complete if it is cancelled.
     */
    @Test
    fun asyncMethodCancellationTest() {
        assertEquals(
            "The async job should have been cancelled but it completed.",
            false,
            baseCancellationTest(true)
        )
    }
}
