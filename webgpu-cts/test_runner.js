// Copyright 2022 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import { globalTestConfig } from '../third_party/webgpu-cts/src/common/framework/test_config.js';
import { dataCache } from '../third_party/webgpu-cts/src/common/framework/data_cache.js';
import { DefaultTestFileLoader } from '../third_party/webgpu-cts/src/common/internal/file_loader.js';
import { prettyPrintLog } from '../third_party/webgpu-cts/src/common/internal/logging/log_message.js';
import { Logger } from '../third_party/webgpu-cts/src/common/internal/logging/logger.js';
import { parseQuery } from '../third_party/webgpu-cts/src/common/internal/query/parseQuery.js';

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
  socket.addEventListener('message', runCtsTestViaSocket);
}

async function runCtsTestViaSocket(event) {
  let input = JSON.parse(event.data);
  runCtsTest(input['q'], input['w']);
}

dataCache.setStore({
  load: async (path) => {
    return await (await fetch(`/third_party/webgpu-cts/cache/data/${path}`)).text();
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
wrapPromiseWithHeartbeat(GPUShaderModule.prototype, 'compilationInfo');

globalTestConfig.testHeartbeatCallback = sendHeartbeat;
globalTestConfig.noRaceWithRejectOnTimeout = true;

// FXC is very slow to compile unrolled const-eval loops, where the metal shader
// compiler (Intel GPU) is very slow to compile rolled loops. Intel drivers for
// linux may also suffer the same performance issues, so unroll const-eval loops
// if we're not running on Windows.
if (navigator.userAgent.indexOf("Windows") !== -1) {
  globalTestConfig.unrollConstEvalLoops = true;
}

async function runCtsTest(query, use_worker) {
  const workerEnabled = use_worker;
  const worker = workerEnabled ? new TestWorker(false) : undefined;

  const loader = new DefaultTestFileLoader();
  const filterQuery = parseQuery(query);
  const testcases = await loader.loadCases(filterQuery);

  const expectations = [];

  const log = new Logger();

  for (const testcase of testcases) {
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

window.runCtsTest = runCtsTest;
window.setupWebsocket = setupWebsocket
