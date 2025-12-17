package androidx.webgpu

import android.graphics.Color
import android.os.Build
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.webgpu.ApiLevelSkipRule
import androidx.webgpu.GPUColor
import androidx.webgpu.helper.toGPUColor
import org.junit.Assert.assertEquals
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class ExtensionsTest {

    @get:Rule val apiSkipRule = ApiLevelSkipRule()
    private val delta = 1e-6

    @Test
    fun testIntToGpuColor() {
        // Create an integer color for opaque red
        val redInt = Color.argb(255, 255, 0, 0)
        val expectedColor = GPUColor(1.0, 0.0, 0.0, 1.0)
        val actualColor = redInt.toGPUColor()

        assertEquals(expectedColor.r, actualColor.r, delta)
        assertEquals(expectedColor.g, actualColor.g, delta)
        assertEquals(expectedColor.b, actualColor.b, delta)
        assertEquals(expectedColor.a, actualColor.a, delta)
    }

    @Test
    fun testIntToGpuColorWithAlpha() {
        // Create an integer color for semi-transparent green.
        val alphaInt = 128
        val greenInt = Color.argb(alphaInt, 0, 255, 0)
        val expectedColor = GPUColor(0.0, 1.0, 0.0, alphaInt / 255.0)
        val actualColor = greenInt.toGPUColor()

        assertEquals(expectedColor.r, actualColor.r, delta)
        assertEquals(expectedColor.g, actualColor.g, delta)
        assertEquals(expectedColor.b, actualColor.b, delta)
        assertEquals(expectedColor.a, actualColor.a, delta)
    }

    @Test
    @ApiRequirement(minApi = Build.VERSION_CODES.O)
    fun testLongToGpuColor() {
        // Create a long color for opaque blue
        val blueLong = Color.pack(0.0f, 0.0f, 1.0f)
        val color = Color.valueOf(blueLong)

        val expectedR = color.red().toDouble()
        val expectedG = color.green().toDouble()
        val expectedB = color.blue().toDouble()
        val expectedA = color.alpha().toDouble()

        val actualColor = blueLong.toGPUColor()

        assertEquals(expectedR, actualColor.r, delta)
        assertEquals(expectedG, actualColor.g, delta)
        assertEquals(expectedB, actualColor.b, delta)
        assertEquals(expectedA, actualColor.a, delta)
    }
}
