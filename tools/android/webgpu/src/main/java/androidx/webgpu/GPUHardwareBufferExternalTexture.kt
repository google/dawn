package androidx.webgpu

@ExperimentalWebGpuApi
public class GPUHardwareBufferExternalTexture(
  handle: Long,
  public val externalTexture: GPUExternalTexture,
) : GPUHardwareBufferWrapper(handle) {

  override fun close() {
    externalTexture.close()
    super.close()
  }
}
