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

import android.os.Process
import android.system.OsConstants
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.SmallTest
import androidx.test.platform.app.InstrumentationRegistry
import androidx.webgpu.GPU.createGPUInstance
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import androidx.webgpu.helper.initLibrary
import dalvik.system.BaseDexClassLoader
import java.io.File
import java.net.URI
import java.util.concurrent.CountDownLatch
import java.util.concurrent.Executor
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicReference
import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.ExecutorCoroutineDispatcher
import kotlinx.coroutines.Job
import kotlinx.coroutines.asCoroutineDispatcher
import kotlinx.coroutines.cancel
import kotlinx.coroutines.cancelAndJoin
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Assert.assertThrows
import org.junit.Assert.assertTrue
import org.junit.Assume.assumeFalse
import org.junit.Assume.assumeTrue
import org.junit.Test
import org.junit.runner.RunWith

private const val PROBE_CALLBACK_ENTERED = "PROBE_CALLBACK_ENTERED"

private fun testShaderSource(seed: Int): String = """
        @vertex fn vertexMain(@builtin(vertex_index) i : u32) ->
        @builtin(position) vec4f {
            return vec4f(${seed}.0, 0.0, 0.0, 1.0);
        }
        @fragment fn fragmentMain() -> @location(0) vec4f {
            return vec4f(0.0, ${seed}.0, 0.0, 1.0);
        } """

private fun GPUDevice.createAsyncPipeline(
  shaderSeed: Int,
  onResult: () -> Unit,
  onError: (Exception) -> Unit
) {
  val shaderModule = createShaderModule(
    GPUShaderModuleDescriptor(shaderSourceWGSL = GPUShaderSourceWGSL(testShaderSource(shaderSeed)))
  )
  createRenderPipelineAsync(
    descriptor = GPURenderPipelineDescriptor(
      vertex = GPUVertexState(module = shaderModule),
      fragment = GPUFragmentState(
        module = shaderModule,
        targets = arrayOf(GPUColorTargetState(format = TextureFormat.RGBA8Unorm))
      )
    ),
    callbackExecutor = Executor(Runnable::run),
    callback = object : GPURequestCallback<GPURenderPipeline> {
      override fun onResult(result: GPURenderPipeline) {
        result.close()
        shaderModule.close()
        onResult()
      }

      override fun onError(exception: Exception) {
        shaderModule.close()
        onError(exception)
      }
    }
  )
}

internal object CallbackAbortProbe {
  @JvmStatic
  fun main(args: Array<String>) {
    val mode = args.singleOrNull() ?: error("Expected probe mode argument")
    when (mode) {
      "unset",
      "broken-tostring" -> Thread.setDefaultUncaughtExceptionHandler(null)
      "throwing" ->
        Thread.setDefaultUncaughtExceptionHandler { _, _ ->
          throw IllegalArgumentException("Secondary handler failure")
        }
      "throwing-thread" ->
        // Install a no-op default handler so that any fallback to defaultUncaughtExceptionHandler
        // when the per-thread handler throws would exit cleanly with status 0 instead of SIGABRT.
        Thread.setDefaultUncaughtExceptionHandler { _, _ -> }
      else -> error("Unknown probe mode: $mode")
    }

    val callbackEnteredLatch = CountDownLatch(1)
    val callbackError = AtomicReference<Exception>()
    runBlocking {
      createWebGpu().use { childWebGpu ->
        val backendType = childWebGpu.execute { childWebGpu.adapter.getInfo().backendType }
        check(backendType == BackendType.Vulkan) {
          "Expected Vulkan backend in subprocess, but got ${BackendType.toString(backendType)}"
        }
        childWebGpu.execute {
          childWebGpu.device.createAsyncPipeline(
            shaderSeed = 201,
            onResult = {
              if (mode == "throwing-thread") {
                Thread.currentThread().uncaughtExceptionHandler =
                  Thread.UncaughtExceptionHandler { _, _ ->
                    throw IllegalArgumentException("Per-thread handler failure")
                  }
              }
              println(PROBE_CALLBACK_ENTERED)
              System.out.flush()
              callbackEnteredLatch.countDown()
              when (mode) {
                "unset" ->
                  throw IllegalStateException("Uncaught callback failure without default handler")
                "throwing" -> throw IllegalStateException("Primary callback failure")
                "throwing-thread" ->
                  throw IllegalStateException("Primary per-thread callback failure")
                "broken-tostring" ->
                  throw object : RuntimeException("Callback failure with broken toString") {
                    override fun toString(): String {
                      throw IllegalStateException("toString failure")
                    }
                  }
                else -> error("Unknown probe mode: $mode")
              }
            },
            onError = { exception ->
              callbackError.set(exception)
              callbackEnteredLatch.countDown()
            }
          )
        }

        check(callbackEnteredLatch.await(10, TimeUnit.SECONDS)) {
          "Timed out waiting for async callback in subprocess"
        }
        callbackError.get()?.let {
          throw IllegalStateException("Unexpected pipeline error in subprocess", it)
        }

        // Enqueue a sequential follow-up task after callbackEnteredLatch unblocks (when tasks is
        // empty and numJobs == 1 in AsyncWorkerThreadPool) so the same DawnWorker thread runs this
        // callback immediately if DispatchPendingException returns without aborting.
        val followUpLatch = CountDownLatch(1)
        val followUpError = AtomicReference<Exception>()
        childWebGpu.execute {
          childWebGpu.device.createAsyncPipeline(
            shaderSeed = 202,
            onResult = { followUpLatch.countDown() },
            onError = { exception ->
              followUpError.set(exception)
              followUpLatch.countDown()
            }
          )
        }
        check(followUpLatch.await(10, TimeUnit.SECONDS)) {
          "Timed out waiting for follow-up async callback in subprocess"
        }
        followUpError.get()?.let {
          throw IllegalStateException("Unexpected follow-up pipeline error in subprocess", it)
        }
      }
    }
  }
}

@RunWith(AndroidJUnit4::class)
@SmallTest
class AsyncHelperTest {

  private val dispatcher: CoroutineDispatcher = Executors.newSingleThreadExecutor { runnable ->
    Thread(runnable, "Test-WebGPU-Thread")
  }.asCoroutineDispatcher()
  private val testScope = CoroutineScope(dispatcher)
  private var processEventsJob: Job? = null
  private lateinit var device: GPUDevice
  private lateinit var webGpu: WebGpu

  private val BASIC_SHADER = """
        @vertex fn vertexMain(@builtin(vertex_index) i : u32) ->
        @builtin(position) vec4f {
            return vec4f();
        }
        @fragment fn fragmentMain() -> @location(0) vec4f {
            return vec4f();
        } """

  private suspend fun ensureInProcessWebGpu(startPollingLoop: Boolean = true) {
    if (!::webGpu.isInitialized) {
      webGpu = createWebGpu(dispatcher)
      device = webGpu.device
    }
    if (startPollingLoop) {
      if (processEventsJob == null) {
        processEventsJob = testScope.launch {
          webGpu.processEventsLoop()
        }
      }
    } else {
      processEventsJob?.cancelAndJoin()
      processEventsJob = null
    }
  }

  private fun assumeAsyncPipelineUsesWorkerThread() {
    val isVulkan = runBlocking {
      ensureInProcessWebGpu(startPollingLoop = false)
      webGpu.execute {
        webGpu.adapter.getInfo().backendType == BackendType.Vulkan
      }
    }
    assumeTrue("Async pipeline worker threads require the Vulkan backend", isVulkan)
  }

  @After
  fun teardown() {
    runBlocking {
      processEventsJob?.cancelAndJoin()
    }
    if (::webGpu.isInitialized) {
      webGpu.close()
    }
    testScope.cancel()
    (dispatcher as? ExecutorCoroutineDispatcher)?.close()
  }

  @Test
  fun asyncMethodTest() {
    runBlocking {
      ensureInProcessWebGpu(startPollingLoop = true)
      webGpu.execute {
        /* Set up a shader module to support the async call. */
        val shaderModule = device.createShaderModule(
          GPUShaderModuleDescriptor(shaderSourceWGSL = GPUShaderSourceWGSL(""))
        )

        val exception = assertThrowsSuspend(WebGpuException::class.java) {
          /* Call an asynchronous method, converted from a callback pattern by a helper. */
          device.createRenderPipelineAndAwait(
            GPURenderPipelineDescriptor(vertex = GPUVertexState(module = shaderModule))
          ).close()
        }

        assertEquals(
          """Create render pipeline (async) should fail when no shader entry point exists.
                       The result was: ${exception.status}""",
          CreatePipelineAsyncStatus.ValidationError,
          exception.status
        )
      }
    }
  }

  @Test
  fun asyncMethodTestValidationPasses() {
    runBlocking {
      ensureInProcessWebGpu(startPollingLoop = true)
      webGpu.execute {
        /* Set up a valid shader module and descriptor */
        val shaderModule = device.createShaderModule(
          GPUShaderModuleDescriptor(shaderSourceWGSL = GPUShaderSourceWGSL(BASIC_SHADER))
        )

        /* Call an asynchronous method, converted from a callback pattern by a helper. */
        @Suppress("UNUSED_VARIABLE")
        val unused = device.createRenderPipelineAndAwait(
          GPURenderPipelineDescriptor(
            vertex = GPUVertexState(module = shaderModule), fragment = GPUFragmentState(
              module = shaderModule,
              targets = arrayOf(GPUColorTargetState(format = TextureFormat.RGBA8Unorm))
            )
          )
        )

        /* Create render pipeline (async) should pass with a simple shader.. */
      }
    }
  }

  private fun baseCancellationTest(doCancel: Boolean): Boolean {
    val hasReturned = AtomicBoolean(false)

    runBlocking {
      ensureInProcessWebGpu(startPollingLoop = true)
      webGpu.execute {
        val shaderModule = device.createShaderModule(
          GPUShaderModuleDescriptor(shaderSourceWGSL = GPUShaderSourceWGSL(BASIC_SHADER))
        )

        /* Launch the function in a new coroutine, giving us a job handle we can cancel. */
        val job = launch {
          @Suppress("UNUSED_VARIABLE")
          val unused = device.createRenderPipelineAndAwait(
            GPURenderPipelineDescriptor(
              vertex = GPUVertexState(module = shaderModule),
              fragment = GPUFragmentState(
                module = shaderModule,
                targets = arrayOf(GPUColorTargetState(format = TextureFormat.RGBA8Unorm))
              )
            )
          )
          hasReturned.set(true)
        }
        assumeFalse("The job completed before we could test it", hasReturned.get())

        if (doCancel) {
          job.cancel()
        }
        job.join()
      }
    }
    return hasReturned.get()
  }

  /**
   * Test that the async-based job will complete if it's not cancelled.
   */
  @Test
  fun asyncMethodCancellationTestControl() {
    assertEquals(
      "The async job should have completed but it failed to do so.",
      true,
      baseCancellationTest(false)
    )
  }

  /**
   * Test that the async-based job will not complete if it is cancelled.
   */
  @Test
  fun asyncMethodCancellationTest() {
    assertEquals(
      "The async job should have been cancelled but it completed.",
      false,
      baseCancellationTest(true)
    )
  }

  @Test
  fun asyncCallbackAttachesThreadWithKernelName() {
    assumeAsyncPipelineUsesWorkerThread()
    for (iteration in 1..2) {
      val latch = CountDownLatch(1)
      val callbackThreadName = AtomicReference<String>()
      val callbackKernelComm = AtomicReference<String>()
      val callbackError = AtomicReference<Throwable>()

      runBlocking {
        ensureInProcessWebGpu(startPollingLoop = false)
        webGpu.execute {
          device.createAsyncPipeline(
            shaderSeed = iteration,
            onResult = {
              try {
                callbackThreadName.set(Thread.currentThread().name)
                callbackKernelComm.set(
                  File("/proc/self/task/${Process.myTid()}/comm").readText().trim()
                )
              } finally {
                latch.countDown()
              }
            },
            onError = { exception ->
              callbackError.set(exception)
              latch.countDown()
            }
          )
        }
      }

      assertTrue("Timed out waiting for async pipeline callback", latch.await(10, TimeUnit.SECONDS))
      assertNull("Unexpected callback error", callbackError.get())
      assertEquals("DawnWorker", callbackThreadName.get())
      assertEquals("DawnWorker", callbackKernelComm.get())
    }
  }

  private fun launchThrowingAsyncPipelineCallback(
    shaderSeed: Int,
    failureMessage: String,
    handlerLatch: CountDownLatch,
    beforeThrow: () -> Unit = {}
  ): AtomicReference<Exception> {
    val pipelineError = AtomicReference<Exception>()
    runBlocking {
      ensureInProcessWebGpu(startPollingLoop = false)
      webGpu.execute {
        device.createAsyncPipeline(
          shaderSeed = shaderSeed,
          onResult = {
            beforeThrow()
            throw IllegalStateException(failureMessage)
          },
          onError = { exception ->
            pipelineError.set(exception)
            handlerLatch.countDown()
          }
        )
      }
    }
    return pipelineError
  }

  private fun awaitAsyncPipelineCallbackThreadName(shaderSeed: Int): String? {
    val latch = CountDownLatch(1)
    val callbackThreadName = AtomicReference<String>()
    val callbackError = AtomicReference<Throwable>()
    runBlocking {
      ensureInProcessWebGpu(startPollingLoop = false)
      webGpu.execute {
        device.createAsyncPipeline(
          shaderSeed = shaderSeed,
          onResult = {
            callbackThreadName.set(Thread.currentThread().name)
            latch.countDown()
          },
          onError = { exception ->
            callbackError.set(exception)
            latch.countDown()
          }
        )
      }
    }
    assertTrue("Follow-up async callback timed out", latch.await(10, TimeUnit.SECONDS))
    assertNull("Unexpected follow-up callback error", callbackError.get())
    return callbackThreadName.get()
  }

  @Test
  fun asyncCallbackForwardsUncaughtExceptionToHandler() {
    assumeAsyncPipelineUsesWorkerThread()
    val expectedMessage = "Uncaught async callback failure"
    val handlerLatch = CountDownLatch(1)
    val caughtThreadName = AtomicReference<String>()
    val caughtThrowable = AtomicReference<Throwable>()
    val previousHandler = Thread.getDefaultUncaughtExceptionHandler()

    Thread.setDefaultUncaughtExceptionHandler { thread, throwable ->
      if (throwable.message == expectedMessage) {
        caughtThreadName.set(thread.name)
        caughtThrowable.set(throwable)
        handlerLatch.countDown()
      } else {
        previousHandler?.uncaughtException(thread, throwable)
      }
    }

    try {
      val pipelineError =
        launchThrowingAsyncPipelineCallback(
          shaderSeed = 101,
          failureMessage = expectedMessage,
          handlerLatch = handlerLatch
        )

      assertTrue(
        "Timed out waiting for UncaughtExceptionHandler",
        handlerLatch.await(10, TimeUnit.SECONDS)
      )
      assertNull("Unexpected pipeline error", pipelineError.get())
      assertEquals("DawnWorker", caughtThreadName.get())
      assertEquals(IllegalStateException::class.java, caughtThrowable.get()?.javaClass)
      assertEquals(expectedMessage, caughtThrowable.get()?.message)
      assertEquals("DawnWorker", awaitAsyncPipelineCallbackThreadName(shaderSeed = 102))
    } finally {
      Thread.setDefaultUncaughtExceptionHandler(previousHandler)
    }
  }

  @Test
  fun asyncCallbackForwardsUncaughtExceptionToThreadHandlerWhenDefaultIsUnset() {
    assumeAsyncPipelineUsesWorkerThread()
    val expectedMessage = "Per-thread uncaught async callback failure"
    val handlerLatch = CountDownLatch(1)
    val caughtThreadName = AtomicReference<String>()
    val caughtThrowable = AtomicReference<Throwable>()
    val workerThreadRef = AtomicReference<Thread>()
    val previousThreadHandlerRef = AtomicReference<Thread.UncaughtExceptionHandler?>()
    val previousDefaultHandler = Thread.getDefaultUncaughtExceptionHandler()

    // While the default handler is null, an unrelated uncaught exception on another thread will not
    // kill the instrumentation process, so a 10 s timeout on handlerLatch is the symptom to look for.
    Thread.setDefaultUncaughtExceptionHandler(null)
    try {
      val pipelineError =
        launchThrowingAsyncPipelineCallback(
          shaderSeed = 103,
          failureMessage = expectedMessage,
          handlerLatch = handlerLatch,
          beforeThrow = {
            val worker = Thread.currentThread()
            workerThreadRef.set(worker)
            previousThreadHandlerRef.set(worker.uncaughtExceptionHandler)
            worker.uncaughtExceptionHandler =
              Thread.UncaughtExceptionHandler { thread, throwable ->
                caughtThreadName.set(thread.name)
                caughtThrowable.set(throwable)
                handlerLatch.countDown()
              }
          }
        )

      assertTrue(
        "Timed out waiting for per-thread UncaughtExceptionHandler",
        handlerLatch.await(10, TimeUnit.SECONDS)
      )
      assertNull("Unexpected pipeline error", pipelineError.get())
      assertEquals("DawnWorker", caughtThreadName.get())
      assertEquals(IllegalStateException::class.java, caughtThrowable.get()?.javaClass)
      assertEquals(expectedMessage, caughtThrowable.get()?.message)
      assertEquals("DawnWorker", awaitAsyncPipelineCallbackThreadName(shaderSeed = 104))
    } finally {
      workerThreadRef.get()?.uncaughtExceptionHandler = previousThreadHandlerRef.get()
      Thread.setDefaultUncaughtExceptionHandler(previousDefaultHandler)
    }
  }

  @Test
  fun asyncCallbackPrefersThreadHandlerOverDefaultHandler() {
    assumeAsyncPipelineUsesWorkerThread()
    val expectedMessage = "Per-thread precedence callback failure"
    val handlerLatch = CountDownLatch(1)
    val defaultInvoked = AtomicBoolean(false)
    val caughtThreadName = AtomicReference<String>()
    val caughtThrowable = AtomicReference<Throwable>()
    val workerThreadRef = AtomicReference<Thread>()
    val previousThreadHandlerRef = AtomicReference<Thread.UncaughtExceptionHandler?>()
    val previousDefaultHandler = Thread.getDefaultUncaughtExceptionHandler()

    Thread.setDefaultUncaughtExceptionHandler { thread, throwable ->
      if (throwable.message == expectedMessage) {
        defaultInvoked.set(true)
        handlerLatch.countDown()
      } else {
        previousDefaultHandler?.uncaughtException(thread, throwable)
      }
    }
    try {
      val pipelineError =
        launchThrowingAsyncPipelineCallback(
          shaderSeed = 105,
          failureMessage = expectedMessage,
          handlerLatch = handlerLatch,
          beforeThrow = {
            val worker = Thread.currentThread()
            workerThreadRef.set(worker)
            previousThreadHandlerRef.set(worker.uncaughtExceptionHandler)
            worker.uncaughtExceptionHandler =
              Thread.UncaughtExceptionHandler { thread, throwable ->
                caughtThreadName.set(thread.name)
                caughtThrowable.set(throwable)
                handlerLatch.countDown()
              }
          }
        )

      assertTrue(
        "Timed out waiting for per-thread UncaughtExceptionHandler",
        handlerLatch.await(10, TimeUnit.SECONDS)
      )
      assertNull("Unexpected pipeline error", pipelineError.get())
      assertEquals(false, defaultInvoked.get())
      assertEquals("DawnWorker", caughtThreadName.get())
      assertEquals(IllegalStateException::class.java, caughtThrowable.get()?.javaClass)
      assertEquals(expectedMessage, caughtThrowable.get()?.message)
      assertEquals("DawnWorker", awaitAsyncPipelineCallbackThreadName(shaderSeed = 106))
    } finally {
      workerThreadRef.get()?.uncaughtExceptionHandler = previousThreadHandlerRef.get()
      Thread.setDefaultUncaughtExceptionHandler(previousDefaultHandler)
    }
  }

  private data class SubprocessResult(val exitCode: Int, val output: String)

  private fun runCallbackAbortProbe(mode: String): SubprocessResult {
    assumeAsyncPipelineUsesWorkerThread()
    val classLoader = checkNotNull(AsyncHelperTest::class.java.classLoader)
    val dexLoader = classLoader as? BaseDexClassLoader
    assumeTrue(
      "Subprocess callback abort test requires libwebgpu_c_bundled.so",
      dexLoader?.findLibrary("webgpu_c_bundled") != null
    )

    val vmNames =
      if (Process.is64Bit()) listOf("dalvikvm64", "dalvikvm") else listOf("dalvikvm32", "dalvikvm")
    val dalvikVm =
      sequenceOf("/apex/com.android.art/bin", "/system/bin")
        .flatMap { dir -> vmNames.asSequence().map { name -> File(dir, name) } }
        .firstOrNull { it.canExecute() }
        ?: throw IllegalStateException("Could not find executable dalvikvm binary")

    val classPathEntries = linkedSetOf<String>()
    val resources = classLoader.getResources("classes.dex")
    while (resources.hasMoreElements()) {
      val rawJarPart = resources.nextElement().path.substringBefore("!/")
      val jarPath =
        runCatching { File(URI(rawJarPart)).absolutePath }
          .getOrElse { rawJarPart.removePrefix("file:") }
      if (jarPath.isNotEmpty()) {
        classPathEntries.add(jarPath)
      }
    }
    assertTrue("Could not resolve test APK classpath for dalvikvm", classPathEntries.isNotEmpty())

    val nativeLibDirs = linkedSetOf<String>()
    fun addNativeLibDir(dirPath: String) {
      if (dirPath.isEmpty()) return
      if ("!/" in dirPath) {
        if (File(dirPath.substringBefore("!/")).isFile) {
          nativeLibDirs.add(dirPath)
        }
        return
      }
      val dir = File(dirPath)
      if (dir.isDirectory) {
        nativeLibDirs.add(dir.absolutePath)
        runCatching { dir.canonicalPath }.getOrNull()?.let { nativeLibDirs.add(it) }
      }
    }
    for (libName in listOf("webgpu_c_bundled", "c++_shared")) {
      val libPath = dexLoader?.findLibrary(libName) ?: continue
      addNativeLibDir(libPath.substringBeforeLast('/', missingDelimiterValue = ""))
    }
    System.getProperty("java.library.path")
      ?.split(File.pathSeparator)
      ?.forEach { addNativeLibDir(it) }
    val nativeLibSearchPath = nativeLibDirs.joinToString(File.pathSeparator)

    val cacheRoot = InstrumentationRegistry.getInstrumentation().targetContext.cacheDir
    val androidDataDir = File(cacheRoot, "webgpu_dalvik_data_${Process.myPid()}_$mode")
    val dalvikCacheDir = File(androidDataDir, "dalvik-cache")
    check(dalvikCacheDir.mkdirs() || dalvikCacheDir.isDirectory) {
      "Failed to create dalvik-cache directory at ${dalvikCacheDir.absolutePath}"
    }

    try {
      val processBuilder =
        ProcessBuilder(
          dalvikVm.absolutePath,
          "-Xcompiler-option",
          "--compiler-filter=verify",
          "-cp",
          classPathEntries.joinToString(File.pathSeparator),
          "-Djava.library.path=$nativeLibSearchPath",
          CallbackAbortProbe::class.java.name,
          mode
        )
      processBuilder.environment()["ANDROID_DATA"] = androidDataDir.absolutePath
      processBuilder.redirectErrorStream(true)

      val process = processBuilder.start()
      val outputBuffer = StringBuffer()
      val readerThread =
        Thread {
            runCatching {
              process.inputStream.bufferedReader().useLines { lines ->
                lines.forEach { line -> outputBuffer.append(line).append('\n') }
              }
            }
          }
          .apply { start() }

      val exited = process.waitFor(30, TimeUnit.SECONDS)
      if (!exited) {
        process.destroyForcibly()
        runCatching { process.inputStream.close() }
      }
      readerThread.join(5_000L)
      val output = outputBuffer.toString()
      assertTrue(
        "Timed out waiting for CallbackAbortProbe($mode) subprocess; output:\n$output",
        exited
      )
      assumeTrue(
        "CallbackAbortProbe($mode) exited before entering callback; output:\n$output",
        output.lineSequence().any { it == PROBE_CALLBACK_ENTERED }
      )
      return SubprocessResult(exitCode = process.exitValue(), output = output)
    } finally {
      androidDataDir.deleteRecursively()
    }
  }

  @Test
  fun asyncCallbackAbortsWhenDefaultUncaughtExceptionHandlerIsUnset() {
    val result = runCallbackAbortProbe("unset")
    assertEquals(
      "Expected SIGABRT when no UncaughtExceptionHandler is configured; output:\n${result.output}",
      128 + OsConstants.SIGABRT,
      result.exitCode
    )
    val expectedAbortLine =
      "Uncaught exception in WebGPU callback: java.lang.IllegalStateException: " +
        "Uncaught callback failure without default handler"
    assertTrue(
      "Expected exact abort message line in subprocess output, got:\n${result.output}",
      result.output.lineSequence().any { it == expectedAbortLine }
    )
  }

  @Test
  fun asyncCallbackAbortsWhenUncaughtExceptionHandlerThrows() {
    val result = runCallbackAbortProbe("throwing")
    assertEquals(
      "Expected SIGABRT when UncaughtExceptionHandler throws; output:\n${result.output}",
      128 + OsConstants.SIGABRT,
      result.exitCode
    )
    val expectedAbortLine =
      "Uncaught exception in WebGPU callback: java.lang.IllegalStateException: " +
        "Primary callback failure (UncaughtExceptionHandler threw: " +
        "java.lang.IllegalArgumentException: Secondary handler failure)"
    assertTrue(
      "Expected exact abort message line in subprocess output, got:\n${result.output}",
      result.output.lineSequence().any { it == expectedAbortLine }
    )
    val primaryTraceIndex = result.output.indexOf("Primary callback failure")
    val secondaryTraceIndex = result.output.indexOf("Secondary handler failure")
    assertTrue(
      "Expected primary exception stack trace before secondary handler failure, got:\n${result.output}",
      primaryTraceIndex in 0 until secondaryTraceIndex
    )
  }

  @Test
  fun asyncCallbackAbortsWhenThreadUncaughtExceptionHandlerThrows() {
    val result = runCallbackAbortProbe("throwing-thread")
    assertEquals(
      "Expected SIGABRT when per-thread UncaughtExceptionHandler throws; output:\n${result.output}",
      128 + OsConstants.SIGABRT,
      result.exitCode
    )
    val expectedAbortLine =
      "Uncaught exception in WebGPU callback: java.lang.IllegalStateException: " +
        "Primary per-thread callback failure (UncaughtExceptionHandler threw: " +
        "java.lang.IllegalArgumentException: Per-thread handler failure)"
    assertTrue(
      "Expected exact abort message line in subprocess output, got:\n${result.output}",
      result.output.lineSequence().any { it == expectedAbortLine }
    )
    val primaryTraceIndex = result.output.indexOf("Primary per-thread callback failure")
    val secondaryTraceIndex = result.output.indexOf("Per-thread handler failure")
    assertTrue(
      "Expected primary exception stack trace before secondary handler failure, got:\n${result.output}",
      primaryTraceIndex in 0 until secondaryTraceIndex
    )
  }

  @Test
  fun asyncCallbackAbortsWhenThrowableToStringThrows() {
    val result = runCallbackAbortProbe("broken-tostring")
    assertEquals(
      "Expected SIGABRT when Throwable.toString() throws; output:\n${result.output}",
      128 + OsConstants.SIGABRT,
      result.exitCode
    )
    val expectedAbortLine =
      "Uncaught exception in WebGPU callback: <Throwable.toString() failed>"
    assertTrue(
      "Expected exact fallback abort message line in subprocess output, got:\n${result.output}",
      result.output.lineSequence().any { it == expectedAbortLine }
    )
  }

  private fun assertSyncRequestAdapterPropagatesCallbackException(expectedMessage: String) {
    initLibrary()
    createGPUInstance().use { instance ->
      val thrown = assertThrows(IllegalStateException::class.java) {
        instance.requestAdapter(
          callbackExecutor = Executor(Runnable::run),
          callback = object : GPURequestCallback<GPUAdapter> {
            override fun onResult(result: GPUAdapter) {
              result.close()
              throw IllegalStateException(expectedMessage)
            }

            override fun onError(exception: Exception) {
              throw IllegalStateException(expectedMessage, exception)
            }
          }
        )
      }
      assertEquals(expectedMessage, thrown.message)
    }
  }

  @Test
  fun syncCallbackPropagatesUncaughtExceptionToCallerWhenDefaultHandlerIsSet() {
    val callerThread = Thread.currentThread()
    val unexpectedDispatch = AtomicReference<Throwable>()
    val previousHandler = Thread.getDefaultUncaughtExceptionHandler()
    Thread.setDefaultUncaughtExceptionHandler { thread, throwable ->
      if (thread === callerThread) {
        unexpectedDispatch.set(throwable)
      } else {
        previousHandler?.uncaughtException(thread, throwable)
      }
    }
    try {
      assertSyncRequestAdapterPropagatesCallbackException("Synchronous callback failure")
      assertNull(
        "Default UncaughtExceptionHandler should not run on caller-attached synchronous callback",
        unexpectedDispatch.get()
      )
    } finally {
      Thread.setDefaultUncaughtExceptionHandler(previousHandler)
    }
  }

  @Test
  fun syncCallbackPropagatesUncaughtExceptionToCallerWhenDefaultHandlerIsUnset() {
    val previousHandler = Thread.getDefaultUncaughtExceptionHandler()
    Thread.setDefaultUncaughtExceptionHandler(null)
    try {
      assertSyncRequestAdapterPropagatesCallbackException("Synchronous callback failure (unset)")
    } finally {
      Thread.setDefaultUncaughtExceptionHandler(previousHandler)
    }
  }
}
