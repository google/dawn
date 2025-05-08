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



// Failure expectations file parsing.



// Returns a dictionary of the tag groups defined in the text
// lines from an expectations file.
function parseTagGroups(lines) {
  const result = {};

  let line_number = 0;
  let group_name = undefined; // Name of the current tag group.
  let group_tags = undefined;
  let state = "start";
  for (const line of lines) {
    line_number++;
    if (state === "start") {
      if (line.startsWith("# BEGIN TAG HEADER")) {
        state = "expect_group_name";
      }
    } else if (state === "expect_group_name") {
      if (line.startsWith("# END TAG HEADER") ||
          line.startsWith("# results:")) {
        return result;
      }
      const match = line.match("^#\\s+(.*?)\\s*$");
      if (match) {
        group_name = match[1];
        group_tags = [];
        state = "in_group";
      } else {
        throw new Error(`missing group tag name, expected on line ${line_number}`);
      }
    } else if (state === "in_group") {
      // This is a little more relaxed than the actual pattern.
      // Allow optional "tags:" and "[" and "]".
      const match = line.match("^#\\s*(tags:)?\\s*\\[?(.*)\\]?");
      if (match) {
        const new_tags = match[2].split(/\s+/).filter(s => s.length > 0 && s !== ']');
        group_tags.push(...new_tags);
      } else {
        console.log(line);
        throw new Error(`invalid tags on line ${line_number}:`);
      }
      if (line.indexOf(']') > 0) {
        result[group_name] = group_tags;
        state = "expect_group_name";
      }
    }
  }
  return result;
}


// Parses an array of text lines from an expectations file.
// Creates an array of objects, keeping only those that satsify
// the predicate.
function parseExpectationLines(lines, pred) {
  const filter = pred ?? ((item) => true);
  let result = new Array();
  const re = new RegExp('^(crbug.com/\\S+)\\s+(\\[(\\s+\\S+)+?\\s*\\])?\\s*(webgpu:\\S+)\\s*\\[\\s*(\\S+)\\s*\\]');
  let line_number = 0;
  for (const line of lines) {
    line_number++;
    let matches = re.exec(line);
    if (matches) {
      let tags = [];
      const tagString = matches[2] ?? '';
      if (tagString !== '') {
        tags = tagString.split(/ +/);
        tags.pop();
        tags.shift();
        tags = tags.sort();
      }
      const pathString = matches[4];
      const bug = matches[1];
      const d = {
        line: line,
        lineNumber: line_number,
        bug: matches[1],
        //tagString,
        tags,
        pathString,
        verdict: matches[5],
      };
      if (pred(d)) {
        result.push(d);
      }
    }
  }
  return result;
}

const kSyntheticRoot = 'webgpu';

// Returns the parent string of a WebGPU CTS path string.
//
// Examples:
//     webgpu:web_platform,copyToTexture,ImageBitmap
// ->  webgpu:web_platform,copyToTexture
//
//     webgpu:web_platform
// ->  webgpu
//
//     a,b,c=[a,1]
// ->  a,b,c
//
//     a,b,c={a:1,b:2}
// ->  a,b
//
//     a,b,texelViewFormat="stencil8"
// ->  a,b
function parentOf(path) {
  const paired = {
    '}': '{',
    '"': '"',
    "'": "'",
    ']': '[',
  };

  let current = path;
  let pruned = false;
  do {
    pruned = false;
    const lastChar = current.substring(current.length-1);
    if (lastChar in paired) {
      const next = current.substring(0, current.lastIndexOf(paired[lastChar]));
      if (next.length === current.length) {
        console.log(`error: unmatched ${lastChar} in test path ${path}`);
        return '';
      }
      current = next;
      pruned = true;
    }
  } while(pruned);
  const cutAt = Math.max(
                    current.lastIndexOf(':'),
                    current.lastIndexOf(','),
                    current.lastIndexOf(';'));
  if (cutAt > 0) {
    const result = current.substring(0,cutAt);
    return result;
  }
  return kSyntheticRoot;
}

// Returns a list of rows where each row has
//    .id
//    .parentId
// And full connectivity between each node to the root.
function prestratify(rows) {
  const hdict = {};
  for (const row of rows) {
    row.id = row.pathString;
  }
  let worklist = rows;
  while(worklist.length > 0) {
    // Link 'current' to its parent.
    let current = worklist.pop();
    hdict[current.id] = current;
    const parentId = parentOf(current.pathString);
    if (parentId !== kSyntheticRoot) {
      current.parentId = parentId;
      if (! (parentId in hdict)) {
        const parent = { id: parentId, pathString: parentId };
        worklist.push(parent);
      }
    } else {
      current.parentId = kSyntheticRoot;
    }
  }
  hdict[kSyntheticRoot] = { id: kSyntheticRoot, pathString: kSyntheticRoot, parentId: '' };
  return Object.values(hdict);
}

function prestratifyExpectationLines(lines, predicate = () => true) {
  return prestratify(parseExpectationLines(lines, predicate));
}

// Sample data below, for manual testing

const sampleLines =
  `
# SharedWorker is not available on Android
crbug.com/dawn/2486 [ android webgpu-shared-worker ] * [ Skip ]

################################################################################
# Temporary Skip Expectations
################################################################################
# The "Skip" expectations in this section are expected to be removable at some
# point.

# SharedImage interop failures on Linux
# Skipped instead of just Crash because of the number of failures
crbug.com/1236130 [ linux ] webgpu:web_platform,canvas,readbackFromWebGPUCanvas:* [ Skip ]
crbug.com/1518248 [ linux ] webgpu:web_platform,copyToTexture,canvas:* [ Skip ]

# web_platform crashes on SwiftShader
# Skipped instead of just Crash because of the number of failures
#crbug.com/1344876 [ mac webgpu-adapter-swiftshader ] webgpu:web_platform,copyToTexture,ImageBitmap:copy_subrect_from_2D_Canvas:* [ Skip ]
#crbug.com/1344876 [ mac webgpu-adapter-swiftshader ] webgpu:web_platform,copyToTexture,ImageBitmap:from_ImageData:* [ Skip ]
#crbug.com/1344876 [ mac webgpu-adapter-swiftshader ] webgpu:web_platform,copyToTexture,ImageBitmap:from_canvas:* [ Skip ]
#crbug.com/1344876 [ mac webgpu-adapter-swiftshader ] webgpu:web_platform,copyToTexture,canvas:copy_contents_from_2d_context_canvas:* [ Skip ]
#crbug.com/1344876 [ mac webgpu-adapter-swiftshader ] webgpu:web_platform,copyToTexture,canvas:copy_contents_from_gl_context_canvas:* [ Skip ]
#crbug.com/1344876 [ webgpu-adapter-swiftshader win ] webgpu:web_platform,copyToTexture,ImageBitmap:copy_subrect_from_2D_Canvas:* [ Skip ]
#crbug.com/1344876 [ webgpu-adapter-swiftshader win ] webgpu:web_platform,copyToTexture,ImageBitmap:from_ImageData:* [ Skip ]
#crbug.com/1344876 [ webgpu-adapter-swiftshader win ] webgpu:web_platform,copyToTexture,ImageBitmap:from_canvas:* [ Skip ]
crbug.com/1344876 [ webgpu-adapter-swiftshader win ] webgpu:web_platform,copyToTexture,canvas:copy_contents_from_2d_context_canvas:* [ Skip ]
crbug.com/1344876 [ webgpu-adapter-swiftshader win ] webgpu:web_platform,copyToTexture,canvas:copy_contents_from_gl_context_canvas:* [ Skip ]

# This one shows braes and lists as components.
crbug.com/dawn/2500 [ android-14 android-pixel-6 ] webgpu:shader,execution,shader_io,fragment_builtins:inputs,position:nearFar=[0,1];sampleCount=4;interpolation={"type":"linear","sampling":"sample"} [ Failure ]

crbug.com/dawn/0000 [ android-14 android-pixel-6 ] webgpu:shader,madeup,shader_io,fragment_builtins:inputs,position:nearFar=[0,1];sampleCount=4;interpolation={"type":"linear","sampling":"sample"} [ Failure ]


crbug.com/407147670 [ amd-0x67ef mac ] webgpu:shader,execution,expression,call,builtin,texture_utils:readTextureToTexelViews:srcFormat="stencil8";texelViewFormat="stencil8";viewDimension="2d-array";sampleCount=1 [ Failure ]
crbug.com/407147670 [ intel mac ]      webgpu:shader,execution,expression,call,builtin,texture_utils:readTextureToTexelViews:srcFormat="stencil8";texelViewFormat="stencil8";viewDimension="2d-array";sampleCount=1 [ Failure ]
  `.split('\n');


//console.log(JSON.stringify(prestratifyExpectationLines(sampleLines)));
//console.log(prestratifyExpectationLines(sampleLines, (e) => (e.bug.match(/\/0+$/))));
//console.log(prestratifyExpectationLines(sampleLines, (e) => (e.pathString.startsWith('webgpu:shader,execution,expression,call,builtin,texture_utils'))));
//console.log(prestratifyExpectationLines(sampleLines));


const sampleTags = `
# BEGIN TAG HEADER (autogenerated, see validate_tag_consistency.py)
# OS
# tags: [ android android-oreo android-pie android-r android-s android-t
#             android-14
#         chromeos
#         fuchsia
#         linux ubuntu
#         mac highsierra mojave catalina bigsur monterey ventura sonoma sequoia
#         win win8 win10 ]
# Devices
# tags: [ android-nexus-5x android-pixel-2 android-pixel-4
#             android-pixel-6 android-shield-android-tv android-sm-a135m
#             android-sm-a235m android-sm-s926b
#         chromeos-board-amd64-generic chromeos-board-eve chromeos-board-jacuzzi
#             chromeos-board-octopus chromeos-board-volteer
#         fuchsia-board-astro fuchsia-board-sherlock fuchsia-board-qemu-x64 ]
# Platform
# tags: [ desktop
#         mobile ]
# Browser
# tags: [ android-chromium android-webview-instrumentation
#         debug debug-x64
#         release release-x64
#         fuchsia-chrome web-engine-shell
#         lacros-chrome cros-chrome ]
# GPU
# tags: [ amd amd-0x6613 amd-0x679e amd-0x67ef amd-0x6821 amd-0x7340
#         apple apple-apple-m1 apple-apple-m2
#             apple-angle-metal-renderer:-apple-m1
#             apple-angle-metal-renderer:-apple-m2
#         arm
#         google google-0xffff google-0xc0de
#         intel intel-gen-9 intel-gen-12 intel-0xa2e intel-0xd26 intel-0xa011
#               intel-0x3e92 intel-0x3e9b intel-0x4680 intel-0x5912 intel-0x9bc5
#         nvidia nvidia-0xfe9 nvidia-0x1cb3 nvidia-0x2184
#         qualcomm ]
# Architecture
# tags: [ mac-arm64 mac-x86_64 ]
# END TAG HEADER

`.split('\n');

//console.log(parseTagGroups(sampleTags));
