package android.dawn.helper

import android.dawn.*
import android.graphics.Bitmap
import java.nio.ByteBuffer

fun createGpuTextureFromBitmap(device: Device, bitmap: Bitmap): Texture {
    val size = Extent3D(width = bitmap.width, height = bitmap.height)
    return device.createTexture(
        TextureDescriptor(
            size = size,
            format = TextureFormat.RGBA8Unorm,
            usage = TextureUsage.TextureBinding or TextureUsage.CopyDst or
                    TextureUsage.RenderAttachment
        )
    ).also { texture ->
        ByteBuffer.allocateDirect(bitmap.height * bitmap.width * Int.SIZE_BYTES).let { pixels ->
            bitmap.copyPixelsToBuffer(pixels)
            device.queue.writeTexture(
                dataLayout = TextureDataLayout(

                    bytesPerRow = bitmap.width * Int.SIZE_BYTES,
                    rowsPerImage = bitmap.height
                ),
                data = pixels,
                destination = ImageCopyTexture(texture = texture),
                writeSize = size
            )
        }
    }
}

suspend fun createImageFromGpuTexture(device: Device, texture: Texture): Bitmap {
    if (texture.width % 64 > 0) {
        throw DawnException("Texture must be a multiple of 64. Was ${texture.width}")
    }

    val size = texture.width * texture.height * Int.SIZE_BYTES
    val readbackBuffer = device.createBuffer(
        BufferDescriptor(
            size = size.toLong(),
            usage = BufferUsage.CopyDst or BufferUsage.MapRead
        )
    )
    device.queue.submit(arrayOf(device.createCommandEncoder().run {
        copyTextureToBuffer(
            source = ImageCopyTexture(texture = texture),
            destination = ImageCopyBuffer(
                layout = TextureDataLayout(
                    offset = 0,
                    bytesPerRow = texture.width * Int.SIZE_BYTES,
                    rowsPerImage = texture.height
                ), buffer = readbackBuffer
            ),
            copySize = Extent3D(width = texture.width, height = texture.height)
        )
        finish()
    }))

    readbackBuffer.mapAsync(MapMode.Read, 0, size.toLong())

    return Bitmap.createBitmap(texture.width, texture.height, Bitmap.Config.ARGB_8888).apply {
        copyPixelsFromBuffer(readbackBuffer.getConstMappedRange(size = readbackBuffer.size))
        readbackBuffer.unmap()
    }
}
