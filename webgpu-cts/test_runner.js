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

import { DefaultTestFileLoader } from '../third_party/webgpu-cts/src/common/internal/file_loader.js';
import { prettyPrintLog } from '../third_party/webgpu-cts/src/common/internal/logging/log_message.js';
import { Logger } from '../third_party/webgpu-cts/src/common/internal/logging/logger.js';
import { parseQuery } from '../third_party/webgpu-cts/src/common/internal/query/parseQuery.js';

import { TestWorker } from '../third_party/webgpu-cts/src/common/runtime/helper/test_worker.js';

var socket;

async function setupWebsocket(port) {
  socket = new WebSocket('ws://127.0.0.1:' + port)
  socket.addEventListener('message', runCtsTestViaSocket);
}

async function runCtsTestViaSocket(event) {
  let input = JSON.parse(event.data);
  runCtsTest(input['q'], input['w']);
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
      const [rec, res] = log.record(name);
      if (worker) {
        await worker.run(rec, name, expectations);
      } else {
        await testcase.run(rec, expectations);
      }

      socket.send(JSON.stringify({'s': res.status,
                                  'l': (res.logs ?? []).map(prettyPrintLog)}));
    };
    await wpt_fn();
  }
}

window.runCtsTest = runCtsTest;
window.setupWebsocket = setupWebsocket
