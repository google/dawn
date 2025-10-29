package androidx.webgpu

import androidx.webgpu.helper.DawnException
import androidx.webgpu.helper.createBitmap
import kotlin.test.assertFailsWith
import kotlinx.coroutines.test.runTest
import org.junit.Test
import org.mockito.Mockito.doReturn
import org.mockito.Mockito.spy
import org.mockito.kotlin.mock
import org.mockito.kotlin.whenever

class TexturesTest {

    @Test
    @kotlinx.coroutines.ExperimentalCoroutinesApi
    fun testBadWidthIsCaught() = runTest {
        val mockDevice = mock<GPUDevice>()
        val partialMockTexture = spy(GPUTexture(0))

        doReturn(65).whenever(partialMockTexture).width
        assertFailsWith<DawnException>{
            partialMockTexture.createBitmap(mockDevice)
        }
    }
}
