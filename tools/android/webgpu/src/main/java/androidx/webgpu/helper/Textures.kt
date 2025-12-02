@file:JvmName("TexturesUtils")

package androidx.webgpu.helper

import android.graphics.Bitmap
import androidx.webgpu.*
import java.nio.ByteBuffer

public fun Bitmap.createGpuTexture(device: GPUDevice): GPUTexture {
    val size = GPUExtent3D(width = width, height = height)
    return device.createTexture(
        GPUTextureDescriptor(
            size = size,
            format = TextureFormat.RGBA8Unorm,
            usage = TextureUsage.TextureBinding or TextureUsage.CopyDst or
                    TextureUsage.RenderAttachment
        )
    ).also { texture ->
        ByteBuffer.allocateDirect(height * width * Int.SIZE_BYTES).let { pixels ->
            copyPixelsToBuffer(pixels)
            device.queue.writeTexture(
                dataLayout = GPUTexelCopyBufferLayout(
                    bytesPerRow = width * Int.SIZE_BYTES,
                    rowsPerImage = height
                ),
                data = pixels,
                destination = GPUTexelCopyTextureInfo(texture = texture),
                writeSize = size
            )
        }
    }
}

public suspend fun GPUTexture.createBitmap(device: GPUDevice): Bitmap {
    if (width % 64 > 0) {
        throw DawnException("Texture must be a multiple of 64. Was ${width}")
    }

    val size = width * height * Int.SIZE_BYTES
    val readbackBuffer = device.createBuffer(
        GPUBufferDescriptor(
            size = size.toLong(),
            usage = BufferUsage.CopyDst or BufferUsage.MapRead
        )
    )
    device.queue.submit(arrayOf(device.createCommandEncoder().let {
        it.copyTextureToBuffer(
            source = GPUTexelCopyTextureInfo(texture = this),
            destination = GPUTexelCopyBufferInfo(
                layout = GPUTexelCopyBufferLayout(
                    offset = 0,
                    bytesPerRow = width * Int.SIZE_BYTES,
                    rowsPerImage = height
                ), buffer = readbackBuffer
            ),
            copySize = GPUExtent3D(width = width, height = height)
        )
        it.finish()
    }))

    readbackBuffer.mapAndAwait(MapMode.Read, 0, size.toLong())

    return Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888).apply {
        copyPixelsFromBuffer(readbackBuffer.getConstMappedRange(size = readbackBuffer.size))
        readbackBuffer.unmap()
    }
}
