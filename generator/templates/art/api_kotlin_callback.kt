// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package {{ kotlin_package }}

import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlinx.coroutines.suspendCancellableCoroutine

/**
 * A generic callback interface for asynchronous GPU requests.
 * @param T The type of the successful result object.
 */
public interface GPURequestCallback<T> {
  public fun onResult(result: T)
  public fun onError(exception: Exception)
}

public suspend fun <T> awaitGPURequest(
  block: (callback: GPURequestCallback<T>) -> Unit,
): T = suspendCancellableCoroutine { continuation ->
  block(object : GPURequestCallback<T> {
    override fun onResult(result: T) {
      if (continuation.isActive) continuation.resume(result)
    }
    override fun onError(exception: Exception) {
      if (continuation.isActive) continuation.resumeWithException(exception)
    }
  })
}

/**
 * Common Base Class for all JNI Runnables.
 *
 * This handles the "Transport Layer" errors. If the native side reports a non-Success
 * status (like DeviceLost or Unknown), this class handles it immediately.
 *
 * @param T The type expected by the callback.
 */
internal abstract class BaseGPURequestRunnable<T>(
  protected val callback: GPURequestCallback<T>,
  protected val status: Int,
  protected val message: String
) : Runnable {

  override fun run() {
    if (status != Status.Success) {
      callback.onError(WebGpuException(status = status, reason = message))
      return
    }

    handleSuccess()
  }

  /**
   * Called only if status == Status.Success.
   * Implementations should check their specific 'result' payload here.
   */
  abstract fun handleSuccess()
}

/**
 * Handles cases where a Result Object (T) is returned.
 */
internal class GPURequestCallbackRunnable<T>(
  callback: GPURequestCallback<T>,
  status: Int,
  message: String,
  private val result: T?,
) : BaseGPURequestRunnable<T>(callback, status, message) {

  override fun handleSuccess() {
    if (result == null) {
      callback.onError(WebGpuException(status = status, reason = "Null value returned"))
    } else {
      callback.onResult(result)
    }
  }
}

/**
 * Handles cases where the native function returns void (Kotlin Unit).
 */
internal class GPURequestCallbackVoidRunnable(
  callback: GPURequestCallback<Unit>,
  status: Int,
  message: String,
) : BaseGPURequestRunnable<Unit>(callback, status, message) {

  override fun handleSuccess() {
    callback.onResult(Unit)
  }
}

/**
 * Handles cases where the "Success" payload is actually an Error Code integer.
 */
internal class GPURequestCallbackErrorTypeRunnable(
  callback: GPURequestCallback<@ErrorType Int>,
  status: Int,
  private val type: @ErrorType Int,
  message: String,
) : BaseGPURequestRunnable<Int>(callback, status, message) {

  override fun handleSuccess() {
    if (type != ErrorType.NoError) {
      callback.onError(WebGpuRuntimeException.create(type, message))
    } else {
      callback.onResult(type)
    }
  }
}