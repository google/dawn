{{{
  // Helper functions for code generation
  globalThis.html5_gpu = {
    makeImportExport: (snake_case) => {
      return `
LibraryHTML5WebGPU.emscripten_webgpu_import_${snake_case}__deps = ['$WebGPU', '$JsValStore'];
LibraryHTML5WebGPU.emscripten_webgpu_import_${snake_case} = (ptr) =>
  WebGPU._tableInsert(JsValStore.get(ptr));

LibraryHTML5WebGPU.emscripten_webgpu_export_${snake_case}__deps = ['$WebGPU', '$JsValStore'];
LibraryHTML5WebGPU.emscripten_webgpu_export_${snake_case} = (ptr) =>
  JsValStore.add(WebGPU._tableGet(ptr));`
    },
  };
  null;
}}}


var LibraryHTML5WebGPU = {
  $JsValStore: {
    values: {},
    next_id: 1,

    add(js_val) {
      var id;
      do {
        id = JsValStore.next_id++;
        if (JsValStore.next_id > 2147483647) JsValStore.next_id = 1; // Wraparound signed int32.
      } while (id in JsValStore.values);

      JsValStore.values[id] = js_val;
      return id;
    },
    remove(id) {
#if ASSERTIONS
      assert(id in JsValStore.values);
#endif
      delete JsValStore.values[id];
    },
    get(id) {
#if ASSERTIONS
      assert(id === 0 || id in JsValStore.values);
#endif
      return JsValStore.values[id];
    },
  },

  emscripten_webgpu_release_js_handle__deps: ['$JsValStore'],
  emscripten_webgpu_release_js_handle: (id) => JsValStore.remove(id),

  emscripten_webgpu_get_device__deps: ['$WebGPU', 'emwgpuTableInsertDevice', 'wgpuDeviceAddRef'],
  emscripten_webgpu_get_device: () => {
#if ASSERTIONS
    assert(Module['preinitializedWebGPUDevice']);
#endif
    if (WebGPU.preinitializedDeviceId === undefined) {
      var device = Module['preinitializedWebGPUDevice'];
      const { instancePtr, devicePtr } = _emwgpuTableInsertDevice(device);
      WebGPU.preinitializedInstanceId = instancePtr;
      WebGPU.preinitializedDeviceId = devicePtr;
    }
    _wgpuDeviceAddRef(WebGPU.preinitializedDeviceId);
    return WebGPU.preinitializedDeviceId;
  },
};

{{{ html5_gpu.makeImportExport('surface') }}}
{{{ html5_gpu.makeImportExport('swap_chain') }}}

{{{ html5_gpu.makeImportExport('device') }}}
{{{ html5_gpu.makeImportExport('queue') }}}

{{{ html5_gpu.makeImportExport('command_buffer') }}}
{{{ html5_gpu.makeImportExport('command_encoder') }}}
{{{ html5_gpu.makeImportExport('render_pass_encoder') }}}
{{{ html5_gpu.makeImportExport('compute_pass_encoder') }}}

{{{ html5_gpu.makeImportExport('bind_group') }}}
{{{ html5_gpu.makeImportExport('buffer') }}}
{{{ html5_gpu.makeImportExport('sampler') }}}
{{{ html5_gpu.makeImportExport('texture') }}}
{{{ html5_gpu.makeImportExport('texture_view') }}}
{{{ html5_gpu.makeImportExport('query_set') }}}

{{{ html5_gpu.makeImportExport('bind_group_layout') }}}
{{{ html5_gpu.makeImportExport('pipeline_layout') }}}
{{{ html5_gpu.makeImportExport('render_pipeline') }}}
{{{ html5_gpu.makeImportExport('compute_pipeline') }}}
{{{ html5_gpu.makeImportExport('shader_module') }}}

{{{ html5_gpu.makeImportExport('render_bundle_encoder') }}}
{{{ html5_gpu.makeImportExport('render_bundle') }}}

mergeInto(LibraryManager.library, LibraryHTML5WebGPU);
