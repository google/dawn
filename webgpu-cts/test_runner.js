// Copyright 2022 The Dawn & Tint Authors
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

import { globalTestConfig } from '../third_party/webgpu-cts/src/common/framework/test_config.js';
import { dataCache } from '../third_party/webgpu-cts/src/common/framework/data_cache.js';
import { DefaultTestFileLoader } from '../third_party/webgpu-cts/src/common/internal/file_loader.js';
import { prettyPrintLog } from '../third_party/webgpu-cts/src/common/internal/logging/log_message.js';
import { Logger } from '../third_party/webgpu-cts/src/common/internal/logging/logger.js';
import { parseQuery } from '../third_party/webgpu-cts/src/common/internal/query/parseQuery.js';
import { parseSearchParamLikeWithCTSOptions } from '../third_party/webgpu-cts/src/common/runtime/helper/options.js';
import { setDefaultRequestAdapterOptions } from '../third_party/webgpu-cts/src/common/util/navigator_gpu.js';

import { TestWorker } from '../third_party/webgpu-cts/src/common/runtime/helper/test_worker.js';

// The Python-side websockets library has a max payload size of 72638. Set the
// max allowable logs size in a single payload to a bit less than that.
const LOGS_MAX_BYTES = 72000;

var socket;

// Returns a wrapper around `fn` which gets called at most once every `intervalMs`.
// If the wrapper is called when `fn` was called too recently, `fn` is scheduled to
// be called later in the future after the interval passes.
// Returns [ wrappedFn, {start, stop}] where wrappedFn is the rate-limited function,
// and start/stop control whether or not the function is enabled. If it is stopped, calls
// to the fn will no-op. If it is started, calls will be rate-limited, starting from
// the time `start` is called.
function rateLimited(fn, intervalMs) {
  let last = undefined;
  let timer = undefined;
  const wrappedFn = (...args) => {
    if (last === undefined) {
      // If the function is not enabled, return.
      return;
    }
    // Get the current time as a number.
    const now = +new Date();
    const diff = now - last;
    if (diff >= intervalMs) {
      // Clear the timer, if there was one. This could happen if a timer
      // is scheduled, but it never runs due to long-running synchronous
      // code.
      if (timer) {
        clearTimeout(timer);
        timer = undefined;
      }

      // Call the function.
      last = now;
      fn(...args);
    } else if (timer === undefined) {
      // Otherwise, we have called `fn` too recently.
      // Schedule a future call.
      timer = setTimeout(() => {
        // Clear the timer to indicate nothing is scheduled.
        timer = undefined;
        last = +new Date();
        fn(...args);
      }, intervalMs - diff + 1);
    }
  };
  return [
    wrappedFn,
    {
      start: () => {
        last = +new Date();
      },
      stop: () => {
        last = undefined;
        if (timer) {
          clearTimeout(timer);
          timer = undefined;
        }
      },
    }
  ];
}

function byteSize(s) {
  return new Blob([s]).size;
}

async function setupWebsocket(port) {
  socket = new WebSocket('ws://127.0.0.1:' + port)
  socket.addEventListener('open', () => {
    socket.send('{"type":"CONNECTION_ACK"}');
  });
  socket.addEventListener('message', runCtsTestViaSocket);
}

async function runCtsTestViaSocket(event) {
  let input = JSON.parse(event.data);
  runCtsTest(input['q'], input['w']);
}

dataCache.setStore({
  load: async (path) => {
    if (path.endsWith('.json')) {
      // TODO(bclayton): Remove this once https://github.com/gpuweb/cts/pull/3094 lands and rolls.
      return await (await fetch(`/third_party/webgpu-cts/cache/data/${path}`)).text();
    } else {
      const response = await fetch(`/third_party/webgpu-cts/cache/data/${path}`);
      return new Uint8Array(await response.arrayBuffer());
    }
  }
});

// Make a rate-limited version `sendMessageTestHeartbeat` that executes
// at most once every 500 ms.
const [sendHeartbeat, {
  start: beginHeartbeatScope,
  stop: endHeartbeatScope
}] = rateLimited(sendMessageTestHeartbeat, 500);

function wrapPromiseWithHeartbeat(prototype, key) {
  const old = prototype[key];
  prototype[key] = function (...args) {
    return new Promise((resolve, reject) => {
      // Send the heartbeat both before and after resolve/reject
      // so that the heartbeat is sent ahead of any potentially
      // long-running synchronous code awaiting the Promise.
      old.call(this, ...args)
        .then(val => { sendHeartbeat(); resolve(val) })
        .catch(err => { sendHeartbeat(); reject(err) })
        .finally(sendHeartbeat);
    });
  }
}

wrapPromiseWithHeartbeat(GPU.prototype, 'requestAdapter');
wrapPromiseWithHeartbeat(GPUAdapter.prototype, 'requestAdapterInfo');
wrapPromiseWithHeartbeat(GPUAdapter.prototype, 'requestDevice');
wrapPromiseWithHeartbeat(GPUDevice.prototype, 'createRenderPipelineAsync');
wrapPromiseWithHeartbeat(GPUDevice.prototype, 'createComputePipelineAsync');
wrapPromiseWithHeartbeat(GPUDevice.prototype, 'popErrorScope');
wrapPromiseWithHeartbeat(GPUQueue.prototype, 'onSubmittedWorkDone');
wrapPromiseWithHeartbeat(GPUBuffer.prototype, 'mapAsync');
wrapPromiseWithHeartbeat(GPUShaderModule.prototype, 'getCompilationInfo');

globalTestConfig.testHeartbeatCallback = sendHeartbeat;
globalTestConfig.noRaceWithRejectOnTimeout = true;

// FXC is very slow to compile unrolled const-eval loops, where the metal shader
// compiler (Intel GPU) is very slow to compile rolled loops. Intel drivers for
// linux may also suffer the same performance issues, so unroll const-eval loops
// if we're not running on Windows.
const isWindows = navigator.userAgent.includes("Windows");
if (!isWindows) {
  globalTestConfig.unrollConstEvalLoops = true;
}

// MAINTENANCE_TODO(gman): remove use_worker since you can use worker=1 instead
async function runCtsTest(queryString, use_worker) {
  const { queries, options } = parseSearchParamLikeWithCTSOptions(queryString);
  const workerEnabled = use_worker || options.worker;
  const worker = workerEnabled ? new TestWorker(options) : undefined;

  const loader = new DefaultTestFileLoader();
  const filterQuery = parseQuery(queries[0]);
  const testcases = Array.from(await loader.loadCases(filterQuery));

  if (testcases.length === 0) {
    sendMessageInfraFailure('Did not find matching test');
    return;
  }
  if (testcases.length !== 1) {
    sendMessageInfraFailure('Found more than 1 test for given query');
    return;
  }
  const testcase = testcases[0];

  const { compatibility, powerPreference } = options;
  globalTestConfig.compatibility = compatibility;
  if (powerPreference || compatibility) {
    setDefaultRequestAdapterOptions({
      ...(powerPreference && { powerPreference }),
      // MAINTENANCE_TODO(gman): Change this to whatever the option ends up being
      ...(compatibility && { compatibilityMode: true }),
    });
  }

  const expectations = [];

  const log = new Logger();

  const name = testcase.query.toString();

  const wpt_fn = async () => {
    sendMessageTestStarted();
    const [rec, res] = log.record(name);

    beginHeartbeatScope();
    if (worker) {
      await worker.run(rec, name, expectations);
    } else {
      await testcase.run(rec, expectations);
    }
    endHeartbeatScope();

    sendMessageTestStatus(res.status, res.timems);
    sendMessageTestLog(res.logs);
    sendMessageTestFinished();
  };
  await wpt_fn();
}

function splitLogsForPayload(fullLogs) {
  let logPieces = [fullLogs]
  // Split the log pieces until they all are guaranteed to fit into a
  // websocket payload.
  while (true) {
    let tempLogPieces = []
    for (const piece of logPieces) {
      if (byteSize(piece) > LOGS_MAX_BYTES) {
        let midpoint = Math.floor(piece.length / 2);
        tempLogPieces.push(piece.substring(0, midpoint));
        tempLogPieces.push(piece.substring(midpoint));
      } else {
        tempLogPieces.push(piece)
      }
    }
    // Didn't make any changes - all pieces are under the size limit.
    if (logPieces.every((value, index) => value == tempLogPieces[index])) {
      break;
    }
    logPieces = tempLogPieces;
  }
  return logPieces
}

function sendMessageTestStarted() {
  socket.send('{"type":"TEST_STARTED"}');
}

function sendMessageTestHeartbeat() {
  socket.send('{"type":"TEST_HEARTBEAT"}');
}

function sendMessageTestStatus(status, jsDurationMs) {
  socket.send(JSON.stringify({
    'type': 'TEST_STATUS',
    'status': status,
    'js_duration_ms': jsDurationMs
  }));
}

function sendMessageTestLog(logs) {
  splitLogsForPayload((logs ?? []).map(prettyPrintLog).join('\n\n'))
    .forEach((piece) => {
      socket.send(JSON.stringify({
        'type': 'TEST_LOG',
        'log': piece
      }));
    });
}

function sendMessageTestFinished() {
  socket.send('{"type":"TEST_FINISHED"}');
}

function sendMessageInfraFailure(message) {
  socket.send(JSON.stringify({
    'type': 'INFRA_FAILURE',
    'message': message,
  }));
}

window.runCtsTest = runCtsTest;
window.setupWebsocket = setupWebsocket
