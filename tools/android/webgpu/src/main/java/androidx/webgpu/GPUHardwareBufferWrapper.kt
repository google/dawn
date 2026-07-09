package androidx.webgpu

import android.os.ParcelFileDescriptor

/**
 * Base abstract wrapper for imported hardware buffers.
 * Manages the native handle and synchronization fences for accessing the buffer.
 */
public abstract class GPUHardwareBufferWrapper(
  private val handle: Long
) : AutoCloseable {

  /**
   * Begins access to the hardware buffer, passing any wait fences to the native layer.
   * @param fences Array of GPUSyncFence objects to wait on before accessing the buffer.
   */
  public fun beginAccess(fences: Array<GPUSyncFence>?) {
    val fds = fences?.mapNotNull { it.getParcelFileDescriptor()?.fd }?.toIntArray()
    nativeBeginAccess(handle, fds)
  }

  /**
   * Ends access to the hardware buffer, returning a signal fence from the native layer.
   * @return GPUSyncFence that will be signaled when WebGPU is done with the buffer.
   */
  public fun endAccess(): GPUSyncFence? {
    val fd = nativeEndAccess(handle)
    if (fd < 0) return null
    // C++ returns a newly dup()d integer. We adopt it here to explicitly transfer
    // ownership to Kotlin/Java so the JVM garbage collector will safely close it.
    val pfd = ParcelFileDescriptor.adoptFd(fd)
    return GPUSyncFence(pfd)
  }

  override fun close() {
    nativeDestroy(handle)
  }

  protected external fun nativeBeginAccess(handle: Long, fences: IntArray?)
  protected external fun nativeEndAccess(handle: Long): Int
  protected external fun nativeDestroy(handle: Long)
}