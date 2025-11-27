package androidx.webgpu

import android.os.Build
import org.junit.Assume
import org.junit.rules.TestRule
import org.junit.runner.Description
import org.junit.runners.model.Statement

class ApiLevelSkipRule : TestRule {
  override fun apply(base: Statement, description: Description): Statement {
    val annotation = getAnnotation(description) ?: return base

    val minApi = annotation.minApi
    if (Build.VERSION.SDK_INT >= minApi) {
      return base // API level is sufficient, do not skip.
    }

    // API level is less than minApi. Check if we should still run.
    if (annotation.onlySkipOnEmulator && !EmulatorUtils.isEmulator) {
      return base // Don't skip if onlySkipOnEmulator is true and we are not on an emulator.
    }
    // Otherwise, skip the test.
    return object : Statement() {
      override fun evaluate() {
        Assume.assumeTrue(
          "Skipping test due to API level requirement: " +
            "minApi=${minApi}, deviceApi=${Build.VERSION.SDK_INT}",
          false,
        )
      }
    }
  }

  private fun getAnnotation(description: Description): ApiRequirement? {
    return description.getAnnotation(ApiRequirement::class.java)
      ?: description.testClass?.getAnnotation(ApiRequirement::class.java)
  }
}
