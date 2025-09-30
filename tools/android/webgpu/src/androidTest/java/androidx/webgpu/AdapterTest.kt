package androidx.webgpu

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.SmallTest
import androidx.webgpu.helper.Util
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
@SmallTest
class AdapterTest {
    @Test
    fun adaptorTest() {
        Util  // Hack to force library initialization.

        val instance = createInstance()
        runBlocking {
            val (status1, adapter, message1) = instance.requestAdapter()
            check(status1 == RequestAdapterStatus.Success && adapter != null) {
                message1 ?: "Error requesting the adapter"
            }
            val adapterInfo = adapter.getInfo()

            assertEquals("The backend type should be Vulkan",
                BackendType.Vulkan.value, adapterInfo.backendType.value
            )
        }
    }
}
