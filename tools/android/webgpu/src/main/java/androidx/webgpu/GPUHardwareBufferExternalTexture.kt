package androidx.webgpu

public class GPUHardwareBufferExternalTexture(
  handle: Long,
  public val externalTexture: GPUExternalTexture,
) : GPUHardwareBufferWrapper(handle) {

  override fun close() {
    externalTexture.close()
    super.close()
  }
}
