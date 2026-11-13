/*
 * Copyright 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package androidx.webgpu

import android.os.Build
import androidx.annotation.ChecksSdkIntAtLeast
import org.junit.Assume
import org.junit.rules.TestRule
import org.junit.runner.Description
import org.junit.runners.model.Statement

class ApiLevelSkipRule : TestRule {
  companion object {
    @ChecksSdkIntAtLeast(parameter = 0, lambda = 1)
    inline fun assumeMinApi(minApi: Int, action: () -> Unit) {
      if (Build.VERSION.SDK_INT >= minApi) {
        action()
      } else {
        Assume.assumeTrue(
          "Skipping test: minApi=$minApi, deviceApi=${Build.VERSION.SDK_INT}",
          false,
        )
      }
    }
  }

  override fun apply(base: Statement, description: Description): Statement {
    val annotation = getAnnotation(description) ?: return base

    val minApi = annotation.minApi
    val maxApi = annotation.maxApi
    if (Build.VERSION.SDK_INT in minApi..maxApi) {
      return base // API level is within range, do not skip.
    }

    // API level is outside [minApi, maxApi]. Check if we should still run.
    // Physical devices still require at least API 28 (Android 9) for Vulkan 1.1,
    // which is Dawn's minimum required Vulkan version (kRequiredVulkanVersion).
    if (
      annotation.onlySkipOnEmulator &&
        !EmulatorUtils.isEmulator &&
        Build.VERSION.SDK_INT >= Build.VERSION_CODES.P
    ) {
      return base // Don't skip if onlySkipOnEmulator is true and we are on a Vulkan 1.1+ physical device.
    }
    // Otherwise, skip the test.
    return object : Statement() {
      override fun evaluate() {
        Assume.assumeTrue(
          "Skipping test due to API level requirement: " +
            "minApi=${minApi}, maxApi=${maxApi}, deviceApi=${Build.VERSION.SDK_INT}",
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
