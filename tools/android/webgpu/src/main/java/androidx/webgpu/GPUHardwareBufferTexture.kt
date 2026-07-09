package androidx.webgpu

@ExperimentalWebGpuApi
public class GPUHardwareBufferTexture(
    handle: Long,
    public val texture: GPUTexture,
) : GPUHardwareBufferWrapper(handle) {

    override fun close() {
        texture.close()
        super.close()
    }
}