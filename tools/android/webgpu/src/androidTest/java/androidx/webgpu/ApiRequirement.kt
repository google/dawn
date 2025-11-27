package androidx.webgpu

import kotlin.annotation.Retention
import kotlin.annotation.Target

@Retention(AnnotationRetention.RUNTIME)
@Target(AnnotationTarget.FUNCTION)
annotation class ApiRequirement(
  val minApi: Int,
  val onlySkipOnEmulator: Boolean = false
)