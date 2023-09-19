# Testing Tint

Tint has multiple levels of testing:

* Unit tests, using the Googletest framework.  These are the lowest level tests,
  comprehensively checking functionality of internal functions and classes.
  The test code is inside the Tint source tree in files with names ending in
  `_test.cc`
* End-to-end tests. These test the whole compiler flow, translating source
  shaders to output shaders in various languages, and optionally checking
  the text of diagnostics.  See [Tint end-to-end tests](end-to-end-testing.md).
* WebGPU Conformance Test Suite (CTS). The WebGPU CTS has both validation and
  execution tests, in the `webgpu:shader` hierarchy.
  See https://github.com/gpuweb/cts
  All test can be run in Chrome, and many tests can be run via `dawn.node`,
  the [Dawn bindings for NodeJS](../../src/dawn/node/README.md).
