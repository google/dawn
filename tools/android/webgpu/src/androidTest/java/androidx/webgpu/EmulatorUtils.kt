package androidx.webgpu

import android.os.Build

object EmulatorUtils {
  val isEmulator: Boolean by lazy {
    checkIsEmulator()
  }

  /**
   * Checks if the current Android environment is running on a probable emulator.
   * This method is thread-safe and computes the result only once.
   *
   * @return True if the device is likely an emulator.
   */
  private fun checkIsEmulator(): Boolean {
    // Check ro.kernel.qemu system property.
    val qemuCheck = runCatching {
      val systemPropertyGet = Class.forName("android.os.SystemProperties")
        .getMethod("get", String::class.java, String::class.java)
      "1" == systemPropertyGet.invoke(null, "ro.kernel.qemu", "0")
    }.getOrDefault(false)

    // Hardware check (ranchu, goldfish, cutf_cvm)
    val hardwareCheck = Build.HARDWARE in listOf("ranchu", "goldfish", "cutf_cvm")
    return qemuCheck || hardwareCheck
  }
}
