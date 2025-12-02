package androidx.webgpu

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.SmallTest
import androidx.webgpu.ValidationException
import androidx.webgpu.helper.createWebGpu
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertThrows
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
@SmallTest
class ErrorTest {
    @Test
    /**
     * Test that an invalid parameter raises an error that is converted to a Kotlin exception by
     * the adapter in DawnTestLauncher.
     */
    fun errorTest() {
        runBlocking {
            val webGpu = createWebGpu()
            val device = webGpu.device
            assertThrows(ValidationException::class.java) {
                device.createTexture(
                    GPUTextureDescriptor(
                        usage = TextureUsage.None,
                        size = GPUExtent3D(0)
                    )
                )
            }
        }
    }
}
