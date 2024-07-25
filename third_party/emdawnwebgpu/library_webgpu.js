/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/*
 * Dawn's fork of Emscripten's WebGPU bindings. This will be contributed back to
 * Emscripten after reaching approximately webgpu.h "1.0".
 *
 * IMPORTANT: See //src/emdawnwebgpu/README.md for info on how to use this.
 */

{{{
  if (USE_WEBGPU || !__HAVE_EMDAWNWEBGPU_STRUCT_INFO || !__HAVE_EMDAWNWEBGPU_ENUM_TABLES || !__HAVE_EMDAWNWEBGPU_SIG_INFO) {
    throw new Error("To use Dawn's library_webgpu.js, disable -sUSE_WEBGPU and first include Dawn's library_webgpu_struct_info.js and library_webgpu_enum_tables.js (before library_webgpu.js)");
  }

  // Helper functions for code generation
  globalThis.gpu = {
    makeInitManager: function(type) {
      var mgr = `WebGPU.mgr${type}`;
      return `${mgr} = ${mgr} || new Manager();`;
    },

    makeAddRefRelease: function(type) {
      return `
wgpu${type}AddRef: (id) => WebGPU.mgr${type}.addRef(id),
wgpu${type}Release: (id) => WebGPU.mgr${type}.release(id),`;
    },

    convertSentinelToUndefined: function(name) {
      return `if (${name} == -1) ${name} = undefined;`;
    },

    makeGetBool: function(struct, offset) {
      return `!!(${makeGetValue(struct, offset, 'u32')})`;
    },
    makeGetU32: function(struct, offset) {
      return makeGetValue(struct, offset, 'u32');
    },
    makeGetU64: function(struct, offset) {
      var l = makeGetValue(struct, offset, 'u32');
      var h = makeGetValue(`(${struct} + 4)`, offset, 'u32')
      return `${h} * 0x100000000 + ${l}`
    },
    makeCheck: function(str) {
      if (!ASSERTIONS) return '';
      return `assert(${str});`;
    },
    makeCheckDefined: function(name) {
      return this.makeCheck(`typeof ${name} != "undefined"`);
    },
    makeCheckDescriptor: function(descriptor) {
      // Assert descriptor is non-null, then that its nextInChain is null.
      // For descriptors that aren't the first in the chain (e.g
      // ShaderModuleSPIRVDescriptor), there is no .nextInChain pointer, but
      // instead a ChainedStruct object: .chain. So we need to check if
      // .chain.nextInChain is null. As long as nextInChain and chain are always
      // the first member in the struct, descriptor.nextInChain and
      // descriptor.chain.nextInChain should have the same offset (0) to the
      // descriptor pointer and we can check it to be null.
      var OffsetOfNextInChainMember = 0;
      return this.makeCheck(descriptor) + this.makeCheck(makeGetValue(descriptor, OffsetOfNextInChainMember, '*') + ' === 0');
    },

    // Compile-time table for enum integer values used with templating.
    // Must be in sync with webgpu.h.
    // TODO: Generate this to keep it in sync with webgpu.h
    COPY_STRIDE_UNDEFINED: 0xFFFFFFFF,
    LIMIT_U32_UNDEFINED: 0xFFFFFFFF,
    MIP_LEVEL_COUNT_UNDEFINED: 0xFFFFFFFF,
    ARRAY_LAYER_COUNT_UNDEFINED: 0xFFFFFFFF,
    ...WEBGPU_ENUM_CONSTANT_TABLES,
  };
  null;
}}}

var LibraryWebGPU = {
  $WebGPU__postset: 'WebGPU.initManagers();',
  $WebGPU__deps: ['$stackSave', '$stackRestore', '$stringToUTF8OnStack'],
  $WebGPU: {
    errorCallback: (callback, type, message, userdata) => {
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack(message);
      {{{ makeDynCall('vipp', 'callback') }}}(type, messagePtr, userdata);
      stackRestore(sp);
    },

    initManagers: () => {
      if (WebGPU.mgrDevice) return;

      /** @constructor */
      function Manager() {
        this.objects = {};
        this.nextId = 1;
        this.create = function(object, wrapper = {}) {
          var id = this.nextId++;
          {{{ gpu.makeCheck("typeof this.objects[id] == 'undefined'") }}}
          wrapper.refcount = 1;
          wrapper.object = object;
          this.objects[id] = wrapper;
          return id;
        };
        this.get = function(id) {
          if (!id) return undefined;
          var o = this.objects[id];
          {{{ gpu.makeCheckDefined('o') }}}
          return o.object;
        };
        this.addRef = function(id) {
          var o = this.objects[id];
          {{{ gpu.makeCheckDefined('o') }}}
          o.refcount++;
        };
        this.release = function(id) {
          var o = this.objects[id];
          {{{ gpu.makeCheckDefined('o') }}}
          {{{ gpu.makeCheck('o.refcount > 0') }}}
          o.refcount--;
          if (o.refcount <= 0) {
            delete this.objects[id];
          }
        };
      }

      {{{ gpu.makeInitManager('Surface') }}}
      {{{ gpu.makeInitManager('SwapChain') }}}

      {{{ gpu.makeInitManager('Adapter') }}}
      // TODO: Release() the device's default queue when the device is freed.
      {{{ gpu.makeInitManager('Device') }}}
      {{{ gpu.makeInitManager('Queue') }}}

      {{{ gpu.makeInitManager('CommandBuffer') }}}
      {{{ gpu.makeInitManager('CommandEncoder') }}}
      {{{ gpu.makeInitManager('RenderPassEncoder') }}}
      {{{ gpu.makeInitManager('ComputePassEncoder') }}}

      {{{ gpu.makeInitManager('BindGroup') }}}
      {{{ gpu.makeInitManager('Buffer') }}}
      {{{ gpu.makeInitManager('Sampler') }}}
      {{{ gpu.makeInitManager('Texture') }}}
      {{{ gpu.makeInitManager('TextureView') }}}
      {{{ gpu.makeInitManager('QuerySet') }}}

      {{{ gpu.makeInitManager('BindGroupLayout') }}}
      {{{ gpu.makeInitManager('PipelineLayout') }}}
      {{{ gpu.makeInitManager('RenderPipeline') }}}
      {{{ gpu.makeInitManager('ComputePipeline') }}}
      {{{ gpu.makeInitManager('ShaderModule') }}}

      {{{ gpu.makeInitManager('RenderBundleEncoder') }}}
      {{{ gpu.makeInitManager('RenderBundle') }}}
    },

    makeColor: (ptr) => {
      return {
        "r": {{{ makeGetValue('ptr', 0, 'double') }}},
        "g": {{{ makeGetValue('ptr', 8, 'double') }}},
        "b": {{{ makeGetValue('ptr', 16, 'double') }}},
        "a": {{{ makeGetValue('ptr', 24, 'double') }}},
      };
    },

    makeExtent3D: (ptr) => {
      return {
        "width": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.width) }}},
        "height": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.height) }}},
        "depthOrArrayLayers": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.depthOrArrayLayers) }}},
      };
    },

    makeOrigin3D: (ptr) => {
      return {
        "x": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.x) }}},
        "y": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.y) }}},
        "z": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.z) }}},
      };
    },

    makeImageCopyTexture: (ptr) => {
      {{{ gpu.makeCheck('ptr') }}}
      return {
        "texture": WebGPU.mgrTexture.get(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUImageCopyTexture.texture, '*') }}}),
        "mipLevel": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUImageCopyTexture.mipLevel) }}},
        "origin": WebGPU.makeOrigin3D(ptr + {{{ C_STRUCTS.WGPUImageCopyTexture.origin }}}),
        "aspect": WebGPU.TextureAspect[{{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUImageCopyTexture.aspect) }}}],
      };
    },

    makeTextureDataLayout: (ptr) => {
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      var bytesPerRow = {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTextureDataLayout.bytesPerRow) }}};
      var rowsPerImage = {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTextureDataLayout.rowsPerImage) }}};
      return {
        "offset": {{{ gpu.makeGetU64('ptr', C_STRUCTS.WGPUTextureDataLayout.offset) }}},
        "bytesPerRow": bytesPerRow === {{{ gpu.COPY_STRIDE_UNDEFINED }}} ? undefined : bytesPerRow,
        "rowsPerImage": rowsPerImage === {{{ gpu.COPY_STRIDE_UNDEFINED }}} ? undefined : rowsPerImage,
      };
    },

    makeImageCopyBuffer: (ptr) => {
      {{{ gpu.makeCheck('ptr') }}}
      var layoutPtr = ptr + {{{ C_STRUCTS.WGPUImageCopyBuffer.layout }}};
      var bufferCopyView = WebGPU.makeTextureDataLayout(layoutPtr);
      bufferCopyView["buffer"] = WebGPU.mgrBuffer.get(
        {{{ makeGetValue('ptr', C_STRUCTS.WGPUImageCopyBuffer.buffer, '*') }}});
      return bufferCopyView;
    },

    makePipelineConstants: (constantCount, constantsPtr) => {
      if (!constantCount) return;
      var constants = {};
      for (var i = 0; i < constantCount; ++i) {
        var entryPtr = constantsPtr + {{{ C_STRUCTS.WGPUConstantEntry.__size__ }}} * i;
        var key = UTF8ToString({{{ makeGetValue('entryPtr', C_STRUCTS.WGPUConstantEntry.key, '*') }}});
        constants[key] = {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUConstantEntry.value, 'double') }}};
      }
      return constants;
    },

    makePipelineLayout: (layoutPtr) => {
      if (!layoutPtr) return 'auto';
      return WebGPU.mgrPipelineLayout.get(layoutPtr);
    },

    makeProgrammableStageDescriptor: (ptr) => {
      if (!ptr) return undefined;
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      var desc = {
        "module": WebGPU.mgrShaderModule.get(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUProgrammableStageDescriptor.module, '*') }}}),
        "constants": WebGPU.makePipelineConstants(
          {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUProgrammableStageDescriptor.constantCount) }}},
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUProgrammableStageDescriptor.constants, '*') }}}),
      };
      var entryPointPtr = {{{ makeGetValue('ptr', C_STRUCTS.WGPUProgrammableStageDescriptor.entryPoint, '*') }}};
      if (entryPointPtr) desc["entryPoint"] = UTF8ToString(entryPointPtr);
      return desc;
    },

    makeComputePipelineDesc: (descriptor) => {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}

      var desc = {
        "label": undefined,
        "layout": WebGPU.makePipelineLayout(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPUComputePipelineDescriptor.layout, '*') }}}),
        "compute": WebGPU.makeProgrammableStageDescriptor(
          descriptor + {{{ C_STRUCTS.WGPUComputePipelineDescriptor.compute }}}),
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUComputePipelineDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
      return desc;
    },

    makeRenderPipelineDesc: (descriptor) => {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      function makePrimitiveState(rsPtr) {
        if (!rsPtr) return undefined;
        {{{ gpu.makeCheck('rsPtr') }}}

        // TODO: This small hack assumes that there's only one type that can be in the chain of
        // WGPUPrimitiveState. The correct thing would be to traverse the chain, but unclippedDepth
        // is going to move into the core object soon, so we'll just do this for now. See:
        // https://github.com/webgpu-native/webgpu-headers/issues/212#issuecomment-1682801259
        var nextInChainPtr = {{{ makeGetValue('rsPtr', C_STRUCTS.WGPUPrimitiveState.nextInChain, '*') }}};
        var sType = nextInChainPtr ? {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}} : 0;

        return {
          "topology": WebGPU.PrimitiveTopology[
            {{{ gpu.makeGetU32('rsPtr', C_STRUCTS.WGPUPrimitiveState.topology) }}}],
          "stripIndexFormat": WebGPU.IndexFormat[
            {{{ gpu.makeGetU32('rsPtr', C_STRUCTS.WGPUPrimitiveState.stripIndexFormat) }}}],
          "frontFace": WebGPU.FrontFace[
            {{{ gpu.makeGetU32('rsPtr', C_STRUCTS.WGPUPrimitiveState.frontFace) }}}],
          "cullMode": WebGPU.CullMode[
            {{{ gpu.makeGetU32('rsPtr', C_STRUCTS.WGPUPrimitiveState.cullMode) }}}],
          "unclippedDepth": sType === {{{ gpu.SType.PrimitiveDepthClipControl }}} && {{{ gpu.makeGetBool('nextInChainPtr', C_STRUCTS.WGPUPrimitiveDepthClipControl.unclippedDepth) }}},
        };
      }

      function makeBlendComponent(bdPtr) {
        if (!bdPtr) return undefined;
        return {
          "operation": WebGPU.BlendOperation[
            {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendComponent.operation) }}}],
          "srcFactor": WebGPU.BlendFactor[
            {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendComponent.srcFactor) }}}],
          "dstFactor": WebGPU.BlendFactor[
            {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendComponent.dstFactor) }}}],
        };
      }

      function makeBlendState(bsPtr) {
        if (!bsPtr) return undefined;
        return {
          "alpha": makeBlendComponent(bsPtr + {{{ C_STRUCTS.WGPUBlendState.alpha }}}),
          "color": makeBlendComponent(bsPtr + {{{ C_STRUCTS.WGPUBlendState.color }}}),
        };
      }

      function makeColorState(csPtr) {
        {{{ gpu.makeCheckDescriptor('csPtr') }}}
        var formatInt = {{{ gpu.makeGetU32('csPtr', C_STRUCTS.WGPUColorTargetState.format) }}};
        return formatInt === {{{ gpu.TextureFormat.Undefined }}} ? undefined : {
          "format": WebGPU.TextureFormat[formatInt],
          "blend": makeBlendState({{{ makeGetValue('csPtr', C_STRUCTS.WGPUColorTargetState.blend, '*') }}}),
          "writeMask": {{{ gpu.makeGetU32('csPtr', C_STRUCTS.WGPUColorTargetState.writeMask) }}},
        };
      }

      function makeColorStates(count, csArrayPtr) {
        var states = [];
        for (var i = 0; i < count; ++i) {
          states.push(makeColorState(csArrayPtr + {{{ C_STRUCTS.WGPUColorTargetState.__size__ }}} * i));
        }
        return states;
      }

      function makeStencilStateFace(ssfPtr) {
        {{{ gpu.makeCheck('ssfPtr') }}}
        return {
          "compare": WebGPU.CompareFunction[
            {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.compare) }}}],
          "failOp": WebGPU.StencilOperation[
            {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.failOp) }}}],
          "depthFailOp": WebGPU.StencilOperation[
            {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.depthFailOp) }}}],
          "passOp": WebGPU.StencilOperation[
            {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.passOp) }}}],
        };
      }

      function makeDepthStencilState(dssPtr) {
        if (!dssPtr) return undefined;

        {{{ gpu.makeCheck('dssPtr') }}}
        return {
          "format": WebGPU.TextureFormat[
            {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.format) }}}],
          "depthWriteEnabled": {{{ gpu.makeGetBool('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthWriteEnabled) }}},
          "depthCompare": WebGPU.CompareFunction[
            {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthCompare) }}}],
          "stencilFront": makeStencilStateFace(dssPtr + {{{ C_STRUCTS.WGPUDepthStencilState.stencilFront }}}),
          "stencilBack": makeStencilStateFace(dssPtr + {{{ C_STRUCTS.WGPUDepthStencilState.stencilBack }}}),
          "stencilReadMask": {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.stencilReadMask) }}},
          "stencilWriteMask": {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.stencilWriteMask) }}},
          "depthBias": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBias, 'i32') }}},
          "depthBiasSlopeScale": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBiasSlopeScale, 'float') }}},
          "depthBiasClamp": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBiasClamp, 'float') }}},
        };
      }

      function makeVertexAttribute(vaPtr) {
        {{{ gpu.makeCheck('vaPtr') }}}
        return {
          "format": WebGPU.VertexFormat[
            {{{ gpu.makeGetU32('vaPtr', C_STRUCTS.WGPUVertexAttribute.format) }}}],
          "offset": {{{ gpu.makeGetU64('vaPtr', C_STRUCTS.WGPUVertexAttribute.offset) }}},
          "shaderLocation": {{{ gpu.makeGetU32('vaPtr', C_STRUCTS.WGPUVertexAttribute.shaderLocation) }}},
        };
      }

      function makeVertexAttributes(count, vaArrayPtr) {
        var vas = [];
        for (var i = 0; i < count; ++i) {
          vas.push(makeVertexAttribute(vaArrayPtr + i * {{{ C_STRUCTS.WGPUVertexAttribute.__size__ }}}));
        }
        return vas;
      }

      function makeVertexBuffer(vbPtr) {
        if (!vbPtr) return undefined;
        var stepModeInt = {{{ gpu.makeGetU32('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.stepMode) }}};
        return stepModeInt === {{{ gpu.VertexStepMode.VertexBufferNotUsed }}} ? null : {
          "arrayStride": {{{ gpu.makeGetU64('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.arrayStride) }}},
          "stepMode": WebGPU.VertexStepMode[stepModeInt],
          "attributes": makeVertexAttributes(
            {{{ gpu.makeGetU32('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.attributeCount) }}},
            {{{ makeGetValue('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.attributes, '*') }}}),
        };
      }

      function makeVertexBuffers(count, vbArrayPtr) {
        if (!count) return undefined;

        var vbs = [];
        for (var i = 0; i < count; ++i) {
          vbs.push(makeVertexBuffer(vbArrayPtr + i * {{{ C_STRUCTS.WGPUVertexBufferLayout.__size__ }}}));
        }
        return vbs;
      }

      function makeVertexState(viPtr) {
        if (!viPtr) return undefined;
        {{{ gpu.makeCheckDescriptor('viPtr') }}}
        var desc = {
          "module": WebGPU.mgrShaderModule.get(
            {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.module, '*') }}}),
          "constants": WebGPU.makePipelineConstants(
            {{{ gpu.makeGetU32('viPtr', C_STRUCTS.WGPUVertexState.constantCount) }}},
            {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.constants, '*') }}}),
          "buffers": makeVertexBuffers(
            {{{ gpu.makeGetU32('viPtr', C_STRUCTS.WGPUVertexState.bufferCount) }}},
            {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.buffers, '*') }}}),
          };
        var entryPointPtr = {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.entryPoint, '*') }}};
        if (entryPointPtr) desc["entryPoint"] = UTF8ToString(entryPointPtr);
        return desc;
      }

      function makeMultisampleState(msPtr) {
        if (!msPtr) return undefined;
        {{{ gpu.makeCheckDescriptor('msPtr') }}}
        return {
          "count": {{{ gpu.makeGetU32('msPtr', C_STRUCTS.WGPUMultisampleState.count) }}},
          "mask": {{{ gpu.makeGetU32('msPtr', C_STRUCTS.WGPUMultisampleState.mask) }}},
          "alphaToCoverageEnabled": {{{ gpu.makeGetBool('msPtr', C_STRUCTS.WGPUMultisampleState.alphaToCoverageEnabled) }}},
        };
      }

      function makeFragmentState(fsPtr) {
        if (!fsPtr) return undefined;
        {{{ gpu.makeCheckDescriptor('fsPtr') }}}
        var desc = {
          "module": WebGPU.mgrShaderModule.get(
            {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.module, '*') }}}),
          "constants": WebGPU.makePipelineConstants(
            {{{ gpu.makeGetU32('fsPtr', C_STRUCTS.WGPUFragmentState.constantCount) }}},
            {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.constants, '*') }}}),
          "targets": makeColorStates(
            {{{ gpu.makeGetU32('fsPtr', C_STRUCTS.WGPUFragmentState.targetCount) }}},
            {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.targets, '*') }}}),
          };
        var entryPointPtr = {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.entryPoint, '*') }}};
        if (entryPointPtr) desc["entryPoint"] = UTF8ToString(entryPointPtr);
        return desc;
      }

      var desc = {
        "label": undefined,
        "layout": WebGPU.makePipelineLayout(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.layout, '*') }}}),
        "vertex": makeVertexState(
          descriptor + {{{ C_STRUCTS.WGPURenderPipelineDescriptor.vertex }}}),
        "primitive": makePrimitiveState(
          descriptor + {{{ C_STRUCTS.WGPURenderPipelineDescriptor.primitive }}}),
        "depthStencil": makeDepthStencilState(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.depthStencil, '*') }}}),
        "multisample": makeMultisampleState(
          descriptor + {{{ C_STRUCTS.WGPURenderPipelineDescriptor.multisample }}}),
        "fragment": makeFragmentState(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.fragment, '*') }}}),
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
      return desc;
    },

    fillLimitStruct: (limits, supportedLimitsOutPtr) => {
      var limitsOutPtr = supportedLimitsOutPtr + {{{ C_STRUCTS.WGPUSupportedLimits.limits }}};

      function setLimitValueU32(name, limitOffset) {
        var limitValue = limits[name];
        {{{ makeSetValue('limitsOutPtr', 'limitOffset', 'limitValue', 'i32') }}};
      }
      function setLimitValueU64(name, limitOffset) {
        var limitValue = limits[name];
        {{{ makeSetValue('limitsOutPtr', 'limitOffset', 'limitValue', 'i64') }}};
      }

      setLimitValueU32('maxTextureDimension1D', {{{ C_STRUCTS.WGPULimits.maxTextureDimension1D }}});
      setLimitValueU32('maxTextureDimension2D', {{{ C_STRUCTS.WGPULimits.maxTextureDimension2D }}});
      setLimitValueU32('maxTextureDimension3D', {{{ C_STRUCTS.WGPULimits.maxTextureDimension3D }}});
      setLimitValueU32('maxTextureArrayLayers', {{{ C_STRUCTS.WGPULimits.maxTextureArrayLayers }}});
      setLimitValueU32('maxBindGroups', {{{ C_STRUCTS.WGPULimits.maxBindGroups }}});
      setLimitValueU32('maxBindGroupsPlusVertexBuffers', {{{ C_STRUCTS.WGPULimits.maxBindGroupsPlusVertexBuffers }}});
      setLimitValueU32('maxBindingsPerBindGroup', {{{ C_STRUCTS.WGPULimits.maxBindingsPerBindGroup }}});
      setLimitValueU32('maxDynamicUniformBuffersPerPipelineLayout', {{{ C_STRUCTS.WGPULimits.maxDynamicUniformBuffersPerPipelineLayout }}});
      setLimitValueU32('maxDynamicStorageBuffersPerPipelineLayout', {{{ C_STRUCTS.WGPULimits.maxDynamicStorageBuffersPerPipelineLayout }}});
      setLimitValueU32('maxSampledTexturesPerShaderStage', {{{ C_STRUCTS.WGPULimits.maxSampledTexturesPerShaderStage }}});
      setLimitValueU32('maxSamplersPerShaderStage', {{{ C_STRUCTS.WGPULimits.maxSamplersPerShaderStage }}});
      setLimitValueU32('maxStorageBuffersPerShaderStage', {{{ C_STRUCTS.WGPULimits.maxStorageBuffersPerShaderStage }}});
      setLimitValueU32('maxStorageTexturesPerShaderStage', {{{ C_STRUCTS.WGPULimits.maxStorageTexturesPerShaderStage }}});
      setLimitValueU32('maxUniformBuffersPerShaderStage', {{{ C_STRUCTS.WGPULimits.maxUniformBuffersPerShaderStage }}});
      setLimitValueU32('minUniformBufferOffsetAlignment', {{{ C_STRUCTS.WGPULimits.minUniformBufferOffsetAlignment }}});
      setLimitValueU32('minStorageBufferOffsetAlignment', {{{ C_STRUCTS.WGPULimits.minStorageBufferOffsetAlignment }}});

      setLimitValueU64('maxUniformBufferBindingSize', {{{ C_STRUCTS.WGPULimits.maxUniformBufferBindingSize }}});
      setLimitValueU64('maxStorageBufferBindingSize', {{{ C_STRUCTS.WGPULimits.maxStorageBufferBindingSize }}});

      setLimitValueU32('maxVertexBuffers', {{{ C_STRUCTS.WGPULimits.maxVertexBuffers }}});
      setLimitValueU64('maxBufferSize', {{{ C_STRUCTS.WGPULimits.maxBufferSize }}});
      setLimitValueU32('maxVertexAttributes', {{{ C_STRUCTS.WGPULimits.maxVertexAttributes }}});
      setLimitValueU32('maxVertexBufferArrayStride', {{{ C_STRUCTS.WGPULimits.maxVertexBufferArrayStride }}});
      setLimitValueU32('maxInterStageShaderComponents', {{{ C_STRUCTS.WGPULimits.maxInterStageShaderComponents }}});
      setLimitValueU32('maxInterStageShaderVariables', {{{ C_STRUCTS.WGPULimits.maxInterStageShaderVariables }}});
      setLimitValueU32('maxColorAttachments', {{{ C_STRUCTS.WGPULimits.maxColorAttachments }}});
      setLimitValueU32('maxColorAttachmentBytesPerSample', {{{ C_STRUCTS.WGPULimits.maxColorAttachmentBytesPerSample }}});
      setLimitValueU32('maxComputeWorkgroupStorageSize', {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupStorageSize }}});
      setLimitValueU32('maxComputeInvocationsPerWorkgroup', {{{ C_STRUCTS.WGPULimits.maxComputeInvocationsPerWorkgroup }}});
      setLimitValueU32('maxComputeWorkgroupSizeX', {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeX }}});
      setLimitValueU32('maxComputeWorkgroupSizeY', {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeY }}});
      setLimitValueU32('maxComputeWorkgroupSizeZ', {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeZ }}});
      setLimitValueU32('maxComputeWorkgroupsPerDimension', {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupsPerDimension }}});
    },

    // Maps from enum string back to enum number, for callbacks.
    {{{ WEBGPU_STRING_TO_INT_TABLES }}}

    // Maps from enum number to enum string.
    {{{ WEBGPU_INT_TO_STRING_TABLES }}}
  },

  // Non-method functions

  wgpuGetInstanceFeatures: (featuresPtr) => {
    abort('TODO: wgpuGetInstanceFeatures unimplemented');
    return 0;
  },

  wgpuGetProcAddress: (device, procName) => {
    abort('TODO(#11526): wgpuGetProcAddress unimplemented');
    return 0;
  },

  // *AddRef/*Release

  {{{ gpu.makeAddRefRelease('Surface') }}}
  {{{ gpu.makeAddRefRelease('SwapChain') }}}

  {{{ gpu.makeAddRefRelease('Adapter') }}}
  {{{ gpu.makeAddRefRelease('Device') }}}
  {{{ gpu.makeAddRefRelease('Queue') }}}

  {{{ gpu.makeAddRefRelease('CommandBuffer') }}}
  {{{ gpu.makeAddRefRelease('CommandEncoder') }}}
  {{{ gpu.makeAddRefRelease('RenderPassEncoder') }}}
  {{{ gpu.makeAddRefRelease('ComputePassEncoder') }}}

  {{{ gpu.makeAddRefRelease('BindGroup') }}}
  {{{ gpu.makeAddRefRelease('Buffer') }}}
  {{{ gpu.makeAddRefRelease('Sampler') }}}
  {{{ gpu.makeAddRefRelease('Texture') }}}
  {{{ gpu.makeAddRefRelease('TextureView') }}}
  {{{ gpu.makeAddRefRelease('QuerySet') }}}

  {{{ gpu.makeAddRefRelease('BindGroupLayout') }}}
  {{{ gpu.makeAddRefRelease('PipelineLayout') }}}
  {{{ gpu.makeAddRefRelease('RenderPipeline') }}}
  {{{ gpu.makeAddRefRelease('ComputePipeline') }}}
  {{{ gpu.makeAddRefRelease('ShaderModule') }}}

  {{{ gpu.makeAddRefRelease('RenderBundleEncoder') }}}
  {{{ gpu.makeAddRefRelease('RenderBundle') }}}

  // *Destroy

  wgpuBufferDestroy: (bufferId) => {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];
    {{{ gpu.makeCheckDefined('bufferWrapper') }}}
    if (bufferWrapper.onUnmap) {
      for (var i = 0; i < bufferWrapper.onUnmap.length; ++i) {
        bufferWrapper.onUnmap[i]();
      }
      bufferWrapper.onUnmap = undefined;
    }

    WebGPU.mgrBuffer.get(bufferId).destroy();
  },
  wgpuTextureDestroy: (textureId) => WebGPU.mgrTexture.get(textureId).destroy(),
  wgpuQuerySetDestroy: (querySetId) => WebGPU.mgrQuerySet.get(querySetId).destroy(),

  // wgpuDevice

  wgpuDeviceEnumerateFeatures: (deviceId, featuresOutPtr) => {
    var device = WebGPU.mgrDevice.get(deviceId);
    if (featuresOutPtr !== 0) {
      var offset = 0;
      device.features.forEach(feature => {
        var featureEnumValue = WebGPU.FeatureNameString2Enum[feature];
        {{{ makeSetValue('featuresOutPtr', 'offset', 'featureEnumValue', 'i32') }}};
        offset += 4;
      });
    }
    return device.features.size;
  },

  wgpuDeviceDestroy: (deviceId) => WebGPU.mgrDevice.get(deviceId).destroy(),

  wgpuDeviceGetLimits: (deviceId, limitsOutPtr) => {
    var device = WebGPU.mgrDevice.objects[deviceId].object;
    WebGPU.fillLimitStruct(device.limits, limitsOutPtr);
    return 1;
  },

  wgpuDeviceGetQueue: (deviceId) => {
    var queueId = WebGPU.mgrDevice.objects[deviceId].queueId;
#if ASSERTIONS
    assert(queueId, 'wgpuDeviceGetQueue: queue was missing or null');
#endif
    // Returns a new reference to the existing queue.
    WebGPU.mgrQueue.addRef(queueId);
    return queueId;
  },

  wgpuDeviceHasFeature: (deviceId, featureEnumValue) => {
    var device = WebGPU.mgrDevice.get(deviceId);
    return device.features.has(WebGPU.FeatureName[featureEnumValue]);
  },

  wgpuDevicePushErrorScope: (deviceId, filter) => {
    var device = WebGPU.mgrDevice.get(deviceId);
    device.pushErrorScope(WebGPU.ErrorFilter[filter]);
  },

  wgpuDevicePopErrorScope__deps: ['$callUserCallback'],
  wgpuDevicePopErrorScope: (deviceId, callback, userdata) => {
    var device = WebGPU.mgrDevice.get(deviceId);
    {{{ runtimeKeepalivePush() }}}
    device.popErrorScope().then((gpuError) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        if (!gpuError) {
          {{{ makeDynCall('vipp', 'callback') }}}(
            {{{ gpu.ErrorType.NoError }}}, 0, userdata);
        } else if (gpuError instanceof GPUOutOfMemoryError) {
          {{{ makeDynCall('vipp', 'callback') }}}(
            {{{ gpu.ErrorType.OutOfMemory }}}, 0, userdata);
        } else {
#if ASSERTIONS
          // TODO: Implement GPUInternalError
          assert(gpuError instanceof GPUValidationError);
#endif
          WebGPU.errorCallback(callback, {{{ gpu.ErrorType.Validation }}}, gpuError.message, userdata);
        }
      });
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        // TODO: This can mean either the device was lost or the error scope stack was empty. Figure
        // out how to synthesize the DeviceLost error type. (Could be by simply tracking the error
        // scope depth, but that isn't ideal.)
        WebGPU.errorCallback(callback, {{{ gpu.ErrorType.Unknown }}}, ex.message, userdata);
      });
    });
  },

  wgpuDeviceSetLabel: (deviceId, labelPtr) => {
    var device = WebGPU.mgrDevice.get(deviceId);
    device.label = UTF8ToString(labelPtr);
  },

  wgpuDeviceSetUncapturedErrorCallback__deps: ['$callUserCallback'],
  wgpuDeviceSetUncapturedErrorCallback: (deviceId, callback, userdata) => {
    var device = WebGPU.mgrDevice.get(deviceId);
    device.onuncapturederror = function(ev) {
      // This will skip the callback if the runtime is no longer alive.
      callUserCallback(() => {
        // WGPUErrorType type, const char* message, void* userdata
        var Validation = 0x00000001;
        var OutOfMemory = 0x00000002;
        var type;
#if ASSERTIONS
        assert(typeof GPUValidationError != 'undefined');
        assert(typeof GPUOutOfMemoryError != 'undefined');
#endif
        if (ev.error instanceof GPUValidationError) type = Validation;
        else if (ev.error instanceof GPUOutOfMemoryError) type = OutOfMemory;
        // TODO: Implement GPUInternalError

        WebGPU.errorCallback(callback, type, ev.error.message, userdata);
      });
    };
  },

  // wgpuDeviceCreate*

  wgpuDeviceCreateCommandEncoder: (deviceId, descriptor) => {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {
        "label": undefined,
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUCommandEncoderDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }
    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrCommandEncoder.create(device.createCommandEncoder(desc));
  },

  wgpuDeviceCreateBuffer: (deviceId, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var mappedAtCreation = {{{ gpu.makeGetBool('descriptor', C_STRUCTS.WGPUBufferDescriptor.mappedAtCreation) }}};

    var desc = {
      "label": undefined,
      "usage": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBufferDescriptor.usage) }}},
      "size": {{{ gpu.makeGetU64('descriptor', C_STRUCTS.WGPUBufferDescriptor.size) }}},
      "mappedAtCreation": mappedAtCreation,
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBufferDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU.mgrDevice.get(deviceId);
    var bufferWrapper = {};
    var id = WebGPU.mgrBuffer.create(device.createBuffer(desc), bufferWrapper);
    if (mappedAtCreation) {
      bufferWrapper.mapMode = {{{ gpu.MapMode.Write }}};
      bufferWrapper.onUnmap = [];
    }
    return id;
  },

  wgpuDeviceCreateTexture: (deviceId, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var desc = {
      "label": undefined,
      "size": WebGPU.makeExtent3D(descriptor + {{{ C_STRUCTS.WGPUTextureDescriptor.size }}}),
      "mipLevelCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.mipLevelCount) }}},
      "sampleCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.sampleCount) }}},
      "dimension": WebGPU.TextureDimension[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.dimension) }}}],
      "format": WebGPU.TextureFormat[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.format) }}}],
      "usage": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.usage) }}},
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var viewFormatCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.viewFormatCount) }}};
    if (viewFormatCount) {
      var viewFormatsPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureDescriptor.viewFormats, '*') }}};
      // viewFormatsPtr pointer to an array of TextureFormat which is an enum of size uint32_t
      desc["viewFormats"] = Array.from({{{ makeHEAPView('32', 'viewFormatsPtr', `viewFormatsPtr + viewFormatCount * 4`) }}},
        function(format) { return WebGPU.TextureFormat[format]; });
    }

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrTexture.create(device.createTexture(desc));
  },

  wgpuDeviceCreateSampler: (deviceId, descriptor) => {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}

      desc = {
        "label": undefined,
        "addressModeU": WebGPU.AddressMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeU) }}}],
        "addressModeV": WebGPU.AddressMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeV) }}}],
        "addressModeW": WebGPU.AddressMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeW) }}}],
        "magFilter": WebGPU.FilterMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.magFilter) }}}],
        "minFilter": WebGPU.FilterMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.minFilter) }}}],
        "mipmapFilter": WebGPU.MipmapFilterMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.mipmapFilter) }}}],
        "lodMinClamp": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSamplerDescriptor.lodMinClamp, 'float') }}},
        "lodMaxClamp": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSamplerDescriptor.lodMaxClamp, 'float') }}},
        "compare": WebGPU.CompareFunction[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.compare) }}}],
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSamplerDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrSampler.create(device.createSampler(desc));
  },

  wgpuDeviceCreateBindGroupLayout: (deviceId, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makeBufferEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var typeInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBufferBindingLayout.type) }}};
      if (!typeInt) return undefined;

      return {
        "type": WebGPU.BufferBindingType[typeInt],
        "hasDynamicOffset":
          {{{ gpu.makeGetBool('entryPtr', C_STRUCTS.WGPUBufferBindingLayout.hasDynamicOffset) }}},
        "minBindingSize":
          {{{ gpu.makeGetU64('entryPtr', C_STRUCTS.WGPUBufferBindingLayout.minBindingSize) }}},
      };
    }

    function makeSamplerEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var typeInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUSamplerBindingLayout.type) }}};
      if (!typeInt) return undefined;

      return {
        "type": WebGPU.SamplerBindingType[typeInt],
      };
    }

    function makeTextureEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var sampleTypeInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUTextureBindingLayout.sampleType) }}};
      if (!sampleTypeInt) return undefined;

      return {
        "sampleType": WebGPU.TextureSampleType[sampleTypeInt],
        "viewDimension": WebGPU.TextureViewDimension[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUTextureBindingLayout.viewDimension) }}}],
        "multisampled":
          {{{ gpu.makeGetBool('entryPtr', C_STRUCTS.WGPUTextureBindingLayout.multisampled) }}},
      };
    }

    function makeStorageTextureEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var accessInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.access) }}}
      if (!accessInt) return undefined;

      return {
        "access": WebGPU.StorageTextureAccess[accessInt],
        "format": WebGPU.TextureFormat[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.format) }}}],
        "viewDimension": WebGPU.TextureViewDimension[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.viewDimension) }}}],
      };
    }

    function makeEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      return {
        "binding":
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.binding) }}},
        "visibility":
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.visibility) }}},
        "buffer": makeBufferEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.buffer }}}),
        "sampler": makeSamplerEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.sampler }}}),
        "texture": makeTextureEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.texture }}}),
        "storageTexture": makeStorageTextureEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.storageTexture }}}),
      };
    }

    function makeEntries(count, entriesPtrs) {
      var entries = [];
      for (var i = 0; i < count; ++i) {
        entries.push(makeEntry(entriesPtrs +
            {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.__size__ }}} * i));
      }
      return entries;
    }

    var desc = {
      "entries": makeEntries(
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.entryCount) }}},
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.entries, '*') }}}
      ),
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrBindGroupLayout.create(device.createBindGroupLayout(desc));
  },

  wgpuDeviceCreateBindGroup__deps: ['$readI53FromI64'],
  wgpuDeviceCreateBindGroup: (deviceId, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makeEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var bufferId = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.buffer) }}};
      var samplerId = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.sampler) }}};
      var textureViewId = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.textureView) }}};
#if ASSERTIONS
      assert((bufferId !== 0) + (samplerId !== 0) + (textureViewId !== 0) === 1);
#endif

      var binding = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.binding) }}};

      if (bufferId) {
        var size = {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupEntry.size, 'i53') }}};
        {{{ gpu.convertSentinelToUndefined('size') }}}

        return {
          "binding": binding,
          "resource": {
            "buffer": WebGPU.mgrBuffer.get(bufferId),
            "offset": {{{ gpu.makeGetU64('entryPtr', C_STRUCTS.WGPUBindGroupEntry.offset) }}},
            "size": size
          },
        };
      } else if (samplerId) {
        return {
          "binding": binding,
          "resource": WebGPU.mgrSampler.get(samplerId),
        };
      } else {
        return {
          "binding": binding,
          "resource": WebGPU.mgrTextureView.get(textureViewId),
        };
      }
    }

    function makeEntries(count, entriesPtrs) {
      var entries = [];
      for (var i = 0; i < count; ++i) {
        entries.push(makeEntry(entriesPtrs +
            {{{C_STRUCTS.WGPUBindGroupEntry.__size__}}} * i));
      }
      return entries;
    }

    var desc = {
      "label": undefined,
      "layout": WebGPU.mgrBindGroupLayout.get(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.layout, '*') }}}),
      "entries": makeEntries(
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.entryCount) }}},
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.entries, '*') }}}
      ),
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrBindGroup.create(device.createBindGroup(desc));
  },

  wgpuDeviceCreatePipelineLayout: (deviceId, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}
    var bglCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.bindGroupLayoutCount) }}};
    var bglPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.bindGroupLayouts, '*') }}};
    var bgls = [];
    for (var i = 0; i < bglCount; ++i) {
      bgls.push(WebGPU.mgrBindGroupLayout.get(
        {{{ makeGetValue('bglPtr', `${POINTER_SIZE} * i`, '*') }}}));
    }
    var desc = {
      "label": undefined,
      "bindGroupLayouts": bgls,
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrPipelineLayout.create(device.createPipelineLayout(desc));
  },

  wgpuDeviceCreateQuerySet: (deviceId, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var desc = {
      "type": WebGPU.QueryType[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.type) }}}],
      "count": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.count) }}},
    };

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrQuerySet.create(device.createQuerySet(desc));
  },

  wgpuDeviceCreateRenderBundleEncoder: (deviceId, descriptor) => {
    {{{ gpu.makeCheck('descriptor') }}}

    function makeRenderBundleEncoderDescriptor(descriptor) {
      {{{ gpu.makeCheck('descriptor') }}}

      function makeColorFormats(count, formatsPtr) {
        var formats = [];
        for (var i = 0; i < count; ++i, formatsPtr += 4) {
          // format could be undefined
          formats.push(WebGPU.TextureFormat[{{{ gpu.makeGetU32('formatsPtr', 0) }}}]);
        }
        return formats;
      }

      var desc = {
        "label": undefined,
        "colorFormats": makeColorFormats(
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.colorFormatCount) }}},
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.colorFormats, '*') }}}),
        "depthStencilFormat": WebGPU.TextureFormat[{{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.depthStencilFormat) }}}],
        "sampleCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.sampleCount) }}},
        "depthReadOnly": {{{ gpu.makeGetBool('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.depthReadOnly) }}},
        "stencilReadOnly": {{{ gpu.makeGetBool('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.stencilReadOnly) }}},
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
      return desc;
    }

    var desc = makeRenderBundleEncoderDescriptor(descriptor);
    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrRenderBundleEncoder.create(device.createRenderBundleEncoder(desc));
  },

  wgpuDeviceCreateComputePipeline: (deviceId, descriptor) => {
    var desc = WebGPU.makeComputePipelineDesc(descriptor);
    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrComputePipeline.create(device.createComputePipeline(desc));
  },

  wgpuDeviceCreateComputePipelineAsync__deps: ['$callUserCallback', '$stringToUTF8OnStack'],
  wgpuDeviceCreateComputePipelineAsync: (deviceId, descriptor, callback, userdata) => {
    var desc = WebGPU.makeComputePipelineDesc(descriptor);
    var device = WebGPU.mgrDevice.get(deviceId);
    {{{ runtimeKeepalivePush() }}}
    device.createComputePipelineAsync(desc).then((pipeline) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        var pipelineId = WebGPU.mgrComputePipeline.create(pipeline);
        {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.CreatePipelineAsyncStatus.Success }}}, pipelineId, 0, userdata);
      });
    }, (pipelineError) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        var sp = stackSave();
        var messagePtr = stringToUTF8OnStack(pipelineError.message);
        if (pipelineError.reason === 'validation') {
          {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.CreatePipelineAsyncStatus.ValidationError }}}, 0, messagePtr, userdata);
        } else if (pipelineError.reason === 'internal') {
          {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.CreatePipelineAsyncStatus.InternalError }}}, 0, messagePtr, userdata);
        } else {
          {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.CreatePipelineAsyncStatus.Unknown }}}, 0, messagePtr, userdata);
        }
        stackRestore(sp);
      });
    });
  },

  wgpuDeviceCreateRenderPipeline: (deviceId, descriptor) => {
    var desc = WebGPU.makeRenderPipelineDesc(descriptor);
    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrRenderPipeline.create(device.createRenderPipeline(desc));
  },

  wgpuDeviceCreateRenderPipelineAsync__deps: ['$callUserCallback', '$stringToUTF8OnStack'],
  wgpuDeviceCreateRenderPipelineAsync: (deviceId, descriptor, callback, userdata) => {
    var desc = WebGPU.makeRenderPipelineDesc(descriptor);
    var device = WebGPU.mgrDevice.get(deviceId);
    {{{ runtimeKeepalivePush() }}}
    device.createRenderPipelineAsync(desc).then((pipeline) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        var pipelineId = WebGPU.mgrRenderPipeline.create(pipeline);
        {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.CreatePipelineAsyncStatus.Success }}}, pipelineId, 0, userdata);
      });
    }, (pipelineError) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        var sp = stackSave();
        var messagePtr = stringToUTF8OnStack(pipelineError.message);
        if (pipelineError.reason === 'validation') {
          {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.CreatePipelineAsyncStatus.ValidationError }}}, 0, messagePtr, userdata);
        } else if (pipelineError.reason === 'internal') {
          {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.CreatePipelineAsyncStatus.InternalError }}}, 0, messagePtr, userdata);
        } else {
          {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.CreatePipelineAsyncStatus.Unknown }}}, 0, messagePtr, userdata);
        }
        stackRestore(sp);
      });
    });
  },

  wgpuDeviceCreateShaderModule: (deviceId, descriptor) => {
    {{{ gpu.makeCheck('descriptor') }}}
    var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUShaderModuleDescriptor.nextInChain, '*') }}};
#if ASSERTIONS
    assert(nextInChainPtr !== 0);
#endif
    var sType = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}};

    var desc = {
      "label": undefined,
      "code": "",
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUShaderModuleDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    switch (sType) {
      case {{{ gpu.SType.ShaderModuleSPIRVDescriptor }}}: {
        var count = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUShaderModuleSPIRVDescriptor.codeSize) }}};
        var start = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUShaderModuleSPIRVDescriptor.code, '*') }}};
        var offset = {{{ getHeapOffset('start', 'u32') }}};
#if PTHREADS
        // Chrome can't currently handle a SharedArrayBuffer view here, so make a copy.
        desc["code"] = HEAPU32.slice(offset, offset + count);
#else
        desc["code"] = HEAPU32.subarray(offset, offset + count);
#endif
        break;
      }
      case {{{ gpu.SType.ShaderModuleWGSLDescriptor }}}: {
        var sourcePtr = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUShaderModuleWGSLDescriptor.code, '*') }}};
        if (sourcePtr) {
          desc["code"] = UTF8ToString(sourcePtr);
        }
        break;
      }
#if ASSERTIONS
      default: abort('unrecognized ShaderModule sType');
#endif
    }

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrShaderModule.create(device.createShaderModule(desc));
  },

  // wgpuQuerySet

  wgpuQuerySetGetCount: (querySetId) => {
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    return querySet.count;
  },

  wgpuQuerySetGetType: (querySetId, labelPtr) => {
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    return querySet.type;
  },

  wgpuQuerySetSetLabel: (querySetId, labelPtr) => {
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    querySet.label = UTF8ToString(labelPtr);
  },

  // wgpuQueue

  wgpuQueueSetLabel: (queueId, labelPtr) => {
    var queue = WebGPU.mgrQueue.get(queueId);
    queue.label = UTF8ToString(labelPtr);
  },

  wgpuQueueSubmit: (queueId, commandCount, commands) => {
#if ASSERTIONS
    assert(commands % 4 === 0);
#endif
    var queue = WebGPU.mgrQueue.get(queueId);
    var cmds = Array.from({{{ makeHEAPView(`${POINTER_BITS}`, 'commands', `commands + commandCount * ${POINTER_SIZE}`)}}},
      (id) => WebGPU.mgrCommandBuffer.get(id));
    queue.submit(cmds);
  },

  wgpuQueueOnSubmittedWorkDone__deps: ['$callUserCallback'],
  wgpuQueueOnSubmittedWorkDone: (queueId, callback, userdata) => {
    var queue = WebGPU.mgrQueue.get(queueId);

    {{{ runtimeKeepalivePush() }}}
    queue.onSubmittedWorkDone().then(() => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        {{{ makeDynCall('vip', 'callback') }}}({{{ gpu.QueueWorkDoneStatus.Success }}}, userdata);
      });
    }, () => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        {{{ makeDynCall('vip', 'callback') }}}({{{ gpu.QueueWorkDoneStatus.Error }}}, userdata);
      });
    });
  },

  wgpuQueueWriteBuffer: (queueId, bufferId, bufferOffset, data, size) => {
    var queue = WebGPU.mgrQueue.get(queueId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    // There is a size limitation for ArrayBufferView. Work around by passing in a subarray
    // instead of the whole heap. crbug.com/1201109
    var subarray = HEAPU8.subarray(data, data + size);
    queue.writeBuffer(buffer, bufferOffset, subarray, 0, size);
  },

  wgpuQueueWriteTexture: (queueId,
      destinationPtr, data, dataSize, dataLayoutPtr, writeSizePtr) => {
    var queue = WebGPU.mgrQueue.get(queueId);

    var destination = WebGPU.makeImageCopyTexture(destinationPtr);
    var dataLayout = WebGPU.makeTextureDataLayout(dataLayoutPtr);
    var writeSize = WebGPU.makeExtent3D(writeSizePtr);
    // This subarray isn't strictly necessary, but helps work around an issue
    // where Chromium makes a copy of the entire heap. crbug.com/1134457
    var subarray = HEAPU8.subarray(data, data + dataSize);
    queue.writeTexture(destination, subarray, dataLayout, writeSize);
  },

  // wgpuCommandEncoder

  wgpuCommandEncoderBeginComputePass: (encoderId, descriptor) => {
    var desc;

    function makeComputePassTimestampWrites(twPtr) {
      if (twPtr === 0) return undefined;

      return {
        "querySet": WebGPU.mgrQuerySet.get(
          {{{ makeGetValue('twPtr', C_STRUCTS.WGPUComputePassTimestampWrites.querySet, '*') }}}),
        "beginningOfPassWriteIndex": {{{ gpu.makeGetU32('twPtr', C_STRUCTS.WGPUComputePassTimestampWrites.beginningOfPassWriteIndex) }}},
        "endOfPassWriteIndex": {{{ gpu.makeGetU32('twPtr', C_STRUCTS.WGPUComputePassTimestampWrites.endOfPassWriteIndex) }}},
      };
    }

    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {
        "label": undefined,
        "timestampWrites": makeComputePassTimestampWrites(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPUComputePassDescriptor.timestampWrites, '*') }}}),
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUComputePassDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    }
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    return WebGPU.mgrComputePassEncoder.create(commandEncoder.beginComputePass(desc));
  },

  wgpuCommandEncoderBeginRenderPass: (encoderId, descriptor) => {
    {{{ gpu.makeCheck('descriptor') }}}

    function makeColorAttachment(caPtr) {
      var viewPtr = {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.view) }}};
      if (viewPtr === 0) {
        // view could be undefined.
        return undefined;
      }

      var depthSlice = {{{ makeGetValue('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.depthSlice, 'i32') }}};
      {{{ gpu.convertSentinelToUndefined('depthSlice') }}}

      var loadOpInt = {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.loadOp) }}};
      #if ASSERTIONS
          assert(loadOpInt !== {{{ gpu.LoadOp.Undefined }}});
      #endif

      var storeOpInt = {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.storeOp) }}};
      #if ASSERTIONS
          assert(storeOpInt !== {{{ gpu.StoreOp.Undefined }}});
      #endif

      var clearValue = WebGPU.makeColor(caPtr + {{{ C_STRUCTS.WGPURenderPassColorAttachment.clearValue }}});

      return {
        "view": WebGPU.mgrTextureView.get(viewPtr),
        "depthSlice": depthSlice,
        "resolveTarget": WebGPU.mgrTextureView.get(
          {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.resolveTarget) }}}),
        "clearValue": clearValue,
        "loadOp":  WebGPU.LoadOp[loadOpInt],
        "storeOp": WebGPU.StoreOp[storeOpInt],
      };
    }

    function makeColorAttachments(count, caPtr) {
      var attachments = [];
      for (var i = 0; i < count; ++i) {
        attachments.push(makeColorAttachment(caPtr + {{{ C_STRUCTS.WGPURenderPassColorAttachment.__size__ }}} * i));
      }
      return attachments;
    }

    function makeDepthStencilAttachment(dsaPtr) {
      if (dsaPtr === 0) return undefined;

      return {
        "view": WebGPU.mgrTextureView.get(
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.view) }}}),
        "depthClearValue": {{{ makeGetValue('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthClearValue, 'float') }}},
        "depthLoadOp": WebGPU.LoadOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthLoadOp) }}}],
        "depthStoreOp": WebGPU.StoreOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthStoreOp) }}}],
        "depthReadOnly": {{{ gpu.makeGetBool('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthReadOnly) }}},
        "stencilClearValue": {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilClearValue) }}},
        "stencilLoadOp": WebGPU.LoadOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilLoadOp) }}}],
        "stencilStoreOp": WebGPU.StoreOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilStoreOp) }}}],
        "stencilReadOnly": {{{ gpu.makeGetBool('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilReadOnly) }}},
      };
    }

    function makeRenderPassTimestampWrites(twPtr) {
      if (twPtr === 0) return undefined;

      return {
        "querySet": WebGPU.mgrQuerySet.get(
          {{{ makeGetValue('twPtr', C_STRUCTS.WGPURenderPassTimestampWrites.querySet, '*') }}}),
        "beginningOfPassWriteIndex": {{{ gpu.makeGetU32('twPtr', C_STRUCTS.WGPURenderPassTimestampWrites.beginningOfPassWriteIndex) }}},
        "endOfPassWriteIndex": {{{ gpu.makeGetU32('twPtr', C_STRUCTS.WGPURenderPassTimestampWrites.endOfPassWriteIndex) }}},
      };
    }

    function makeRenderPassDescriptor(descriptor) {
      {{{ gpu.makeCheck('descriptor') }}}
      var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.nextInChain, '*') }}};

      var maxDrawCount = undefined;
      if (nextInChainPtr !== 0) {
        var sType = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}};
#if ASSERTIONS
        assert(sType === {{{ gpu.SType.RenderPassDescriptorMaxDrawCount }}});
        assert(0 === {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.next, '*') }}});
#endif
        var renderPassDescriptorMaxDrawCount = nextInChainPtr;
        {{{ gpu.makeCheckDescriptor('renderPassDescriptorMaxDrawCount') }}}
        maxDrawCount = {{{ gpu.makeGetU64('renderPassDescriptorMaxDrawCount', C_STRUCTS.WGPURenderPassDescriptorMaxDrawCount.maxDrawCount) }}};
      }

      var desc = {
        "label": undefined,
        "colorAttachments": makeColorAttachments(
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderPassDescriptor.colorAttachmentCount) }}},
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.colorAttachments, '*') }}}),
        "depthStencilAttachment": makeDepthStencilAttachment(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.depthStencilAttachment, '*') }}}),
        "occlusionQuerySet": WebGPU.mgrQuerySet.get(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.occlusionQuerySet, '*') }}}),
        "timestampWrites": makeRenderPassTimestampWrites(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.timestampWrites, '*') }}}),
          "maxDrawCount": maxDrawCount,
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

      return desc;
    }

    var desc = makeRenderPassDescriptor(descriptor);

    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    return WebGPU.mgrRenderPassEncoder.create(commandEncoder.beginRenderPass(desc));
  },

  wgpuCommandEncoderClearBuffer: (encoderId, bufferId, offset, size) => {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    {{{ gpu.convertSentinelToUndefined('size') }}}

    var buffer = WebGPU.mgrBuffer.get(bufferId);
    commandEncoder.clearBuffer(buffer, offset, size);
  },

  wgpuCommandEncoderCopyBufferToBuffer: (encoderId, srcId, srcOffset, dstId, dstOffset, size) => {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var src = WebGPU.mgrBuffer.get(srcId);
    var dst = WebGPU.mgrBuffer.get(dstId);
    commandEncoder.copyBufferToBuffer(src, srcOffset, dst, dstOffset, size);
  },

  wgpuCommandEncoderCopyBufferToTexture: (encoderId, srcPtr, dstPtr, copySizePtr) => {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder.copyBufferToTexture(
      WebGPU.makeImageCopyBuffer(srcPtr), WebGPU.makeImageCopyTexture(dstPtr), copySize);
  },

  wgpuCommandEncoderCopyTextureToBuffer: (encoderId, srcPtr, dstPtr, copySizePtr) => {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder.copyTextureToBuffer(
      WebGPU.makeImageCopyTexture(srcPtr), WebGPU.makeImageCopyBuffer(dstPtr), copySize);
  },

  wgpuCommandEncoderCopyTextureToTexture: (encoderId, srcPtr, dstPtr, copySizePtr) => {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder.copyTextureToTexture(
      WebGPU.makeImageCopyTexture(srcPtr), WebGPU.makeImageCopyTexture(dstPtr), copySize);
  },

  wgpuCommandEncoderResolveQuerySet: (encoderId, querySetId, firstQuery, queryCount,
      destinationId, destinationOffset) => {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    var destination = WebGPU.mgrBuffer.get(destinationId);

    commandEncoder.resolveQuerySet(querySet, firstQuery, queryCount, destination, destinationOffset);
  },

  wgpuCommandEncoderWriteTimestamp: (encoderId, querySetId, queryIndex) => {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    commandEncoder.writeTimestamp(querySet, queryIndex);
  },

  wgpuCommandEncoderPushDebugGroup: (encoderId, groupLabelPtr) => {
    var encoder = WebGPU.mgrCommandEncoder.get(encoderId);
    encoder.pushDebugGroup(UTF8ToString(groupLabelPtr));
  },
  wgpuCommandEncoderPopDebugGroup: (encoderId) => {
    var encoder = WebGPU.mgrCommandEncoder.get(encoderId);
    encoder.popDebugGroup();
  },
  wgpuCommandEncoderInsertDebugMarker: (encoderId, markerLabelPtr) => {
    var encoder = WebGPU.mgrCommandEncoder.get(encoderId);
    encoder.insertDebugMarker(UTF8ToString(markerLabelPtr));
  },

  wgpuCommandEncoderFinish: (encoderId, descriptor) => {
    // TODO: Use the descriptor.
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    return WebGPU.mgrCommandBuffer.create(commandEncoder.finish());
  },

  wgpuCommandEncoderSetLabel: (encoderId, labelPtr) => {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    commandEncoder.label = UTF8ToString(labelPtr);
  },

  // wgpuCommandBuffer

  wgpuCommandBufferSetLabel: (commandBufferId, labelPtr) => {
    var commandBuffer = WebGPU.mgrCommandBuffer.get(commandBufferId);
    commandBuffer.label = UTF8ToString(labelPtr);
  },

  // wgpuPipelineLayout

  wgpuPipelineLayoutSetLabel: (pipelineLayoutId, labelPtr) => {
    var pipelineLayout = WebGPU.mgrPipelineLayout.get(pipelineLayoutId);
    pipelineLayout.label = UTF8ToString(labelPtr);
  },

  // wgpuShaderModule

  wgpuShaderModuleGetCompilationInfo__deps: ['$callUserCallback', '$stringToUTF8', '$lengthBytesUTF8', 'malloc', 'free'],
  wgpuShaderModuleGetCompilationInfo: (shaderModuleId, callback, userdata) => {
    var shaderModule = WebGPU.mgrShaderModule.get(shaderModuleId);
    {{{ runtimeKeepalivePush() }}}
    shaderModule.getCompilationInfo().then((compilationInfo) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        var compilationMessagesPtr = _malloc({{{ C_STRUCTS.WGPUCompilationMessage.__size__ }}} * compilationInfo.messages.length);
        var messageStringPtrs = []; // save these to free later
        for (var i = 0; i < compilationInfo.messages.length; ++i) {
          var compilationMessage = compilationInfo.messages[i];
          var compilationMessagePtr = compilationMessagesPtr + {{{ C_STRUCTS.WGPUCompilationMessage.__size__ }}} * i;
          var messageSize = lengthBytesUTF8(compilationMessage.message) + 1;
          var messagePtr = _malloc(messageSize);
          messageStringPtrs.push(messagePtr);
          stringToUTF8(compilationMessage.message, messagePtr, messageSize);
          {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.message, 'messagePtr', '*') }}};
          {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.type, 'WebGPU.Int_CompilationMessageType[compilationMessage.type]', 'i32') }}};
          {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.lineNum, 'compilationMessage.lineNum', 'i64') }}};
          {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.linePos, 'compilationMessage.linePos', 'i64') }}};
          {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.offset, 'compilationMessage.offset', 'i64') }}};
          {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.length, 'compilationMessage.length', 'i64') }}};
          // TODO: Convert JavaScript's UTF-16-code-unit offsets to UTF-8-code-unit offsets.
          // https://github.com/webgpu-native/webgpu-headers/issues/246
          {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.utf16LinePos, 'compilationMessage.linePos', 'i64') }}};
          {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.utf16Offset, 'compilationMessage.offset', 'i64') }}};
          {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.utf16Length, 'compilationMessage.length', 'i64') }}};
        }
        var compilationInfoPtr = _malloc({{{ C_STRUCTS.WGPUCompilationInfo.__size__ }}});
        {{{ makeSetValue('compilationInfoPtr', C_STRUCTS.WGPUCompilationInfo.messageCount, 'compilationInfo.messages.length', '*') }}}
        {{{ makeSetValue('compilationInfoPtr', C_STRUCTS.WGPUCompilationInfo.messages, 'compilationMessagesPtr', '*') }}};

        {{{ makeDynCall('vipp', 'callback') }}}({{{ gpu.CompilationInfoRequestStatus.Success }}}, compilationInfoPtr, userdata);

        messageStringPtrs.forEach((ptr) => {
          _free(ptr);
        });
        _free(compilationMessagesPtr);
        _free(compilationInfoPtr);
      });
    });
  },
  wgpuShaderModuleSetLabel: (shaderModuleId, labelPtr) => {
    var shaderModule = WebGPU.mgrShaderModule.get(shaderModuleId);
    shaderModule.label = UTF8ToString(labelPtr);
  },

  // wgpuComputePipeline

  wgpuComputePipelineGetBindGroupLayout: (pipelineId, groupIndex) => {
    var pipeline = WebGPU.mgrComputePipeline.get(pipelineId);
    return WebGPU.mgrBindGroupLayout.create(pipeline.getBindGroupLayout(groupIndex));
  },
  wgpuComputePipelineSetLabel: (pipelineId, labelPtr) => {
    var pipeline = WebGPU.mgrComputePipeline.get(pipelineId);
    pipeline.label = UTF8ToString(labelPtr);
  },

  // wgpuRenderPipeline

  wgpuRenderPipelineGetBindGroupLayout: (pipelineId, groupIndex) => {
    var pipeline = WebGPU.mgrRenderPipeline.get(pipelineId);
    return WebGPU.mgrBindGroupLayout.create(pipeline.getBindGroupLayout(groupIndex));
  },
  wgpuRenderPipelineSetLabel: (pipelineId, labelPtr) => {
    var pipeline = WebGPU.mgrRenderPipeline.get(pipelineId);
    pipeline.label = UTF8ToString(labelPtr);
  },

  // wgpuBindGroup

  wgpuBindGroupSetLabel: (bindGroupId, labelPtr) => {
    var bindGroup = WebGPU.mgrBindGroup.get(bindGroupId);
    bindGroup.label = UTF8ToString(labelPtr);
  },

  // wgpuBindGroupLayout

  wgpuBindGroupLayoutSetLabel: (bindGroupLayoutId, labelPtr) => {
    var bindGroupLayout = WebGPU.mgrBindGroupLayout.get(bindGroupLayoutId);
    bindGroupLayout.label = UTF8ToString(labelPtr);
  },

  // wgpuBuffer

  // In webgpu.h offset and size are passed in as size_t.
  // And library_webgpu assumes that size_t is always 32bit in emscripten.
  wgpuBufferGetConstMappedRange__deps: ['$warnOnce', 'memalign', 'free'],
  wgpuBufferGetConstMappedRange: (bufferId, offset, size) => {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];
    {{{ gpu.makeCheckDefined('bufferWrapper') }}}

    if (size === 0) warnOnce('getMappedRange size=0 no longer means WGPU_WHOLE_MAP_SIZE');

    {{{ gpu.convertSentinelToUndefined('size') }}}

    var mapped;
    try {
      mapped = bufferWrapper.object.getMappedRange(offset, size);
    } catch (ex) {
#if ASSERTIONS
      err(`wgpuBufferGetConstMappedRange(${offset}, ${size}) failed: ${ex}`);
#endif
      // TODO(kainino0x): Somehow inject a validation error?
      return 0;
    }
    var data = _memalign(16, mapped.byteLength);
    HEAPU8.set(new Uint8Array(mapped), data);
    bufferWrapper.onUnmap.push(() => _free(data));
    return data;
  },

  wgpuBufferGetMapState: (bufferId) => {
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    return WebGPU.Int_BufferMapState[buffer.mapState];
  },

  // In webgpu.h offset and size are passed in as size_t.
  // And library_webgpu assumes that size_t is always 32bit in emscripten.
  wgpuBufferGetMappedRange__deps: ['$warnOnce', 'memalign', 'free'],
  wgpuBufferGetMappedRange: (bufferId, offset, size) => {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];
    {{{ gpu.makeCheckDefined('bufferWrapper') }}}

    if (size === 0) warnOnce('getMappedRange size=0 no longer means WGPU_WHOLE_MAP_SIZE');

    {{{ gpu.convertSentinelToUndefined('size') }}}

    if (bufferWrapper.mapMode !== {{{ gpu.MapMode.Write }}}) {
#if ASSERTIONS
      abort("GetMappedRange called, but buffer not mapped for writing");
#endif
      // TODO(kainino0x): Somehow inject a validation error?
      return 0;
    }

    var mapped;
    try {
      mapped = bufferWrapper.object.getMappedRange(offset, size);
    } catch (ex) {
#if ASSERTIONS
      err(`wgpuBufferGetMappedRange(${offset}, ${size}) failed: ${ex}`);
#endif
      // TODO(kainino0x): Somehow inject a validation error?
      return 0;
    }

    var data = _memalign(16, mapped.byteLength);
    HEAPU8.fill(0, data, mapped.byteLength);
    bufferWrapper.onUnmap.push(() => {
      new Uint8Array(mapped).set(HEAPU8.subarray(data, data + mapped.byteLength));
      _free(data);
    });
    return data;
  },

  // In webgpu.h offset and size are passed in as size_t.
  // And library_webgpu assumes that size_t is always 32bit in emscripten.
  wgpuBufferMapAsync__deps: ['$callUserCallback'],
  wgpuBufferMapAsync: (bufferId, mode, offset, size, callback, userdata) => {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];
    {{{ gpu.makeCheckDefined('bufferWrapper') }}}
    bufferWrapper.mapMode = mode;
    bufferWrapper.onUnmap = [];
    var buffer = bufferWrapper.object;

    {{{ gpu.convertSentinelToUndefined('size') }}}

    // `callback` takes (WGPUBufferMapAsyncStatus status, void * userdata)

    {{{ runtimeKeepalivePush() }}}
    buffer.mapAsync(mode, offset, size).then(() => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        {{{ makeDynCall('vip', 'callback') }}}({{{ gpu.BufferMapAsyncStatus.Success }}}, userdata);
      });
    }, () => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        // TODO(kainino0x): Figure out how to pick other error status values.
        {{{ makeDynCall('vip', 'callback') }}}({{{ gpu.BufferMapAsyncStatus.ValidationError }}}, userdata);
      });
    });
  },

  wgpuBufferGetSize: (bufferId) => {
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    // 64-bit
    return buffer.size;
  },

  wgpuBufferGetUsage: (bufferId) => {
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    return buffer.usage;
  },

  wgpuBufferSetLabel: (bufferId, labelPtr) => {
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    buffer.label = UTF8ToString(labelPtr);
  },

  wgpuBufferUnmap: (bufferId) => {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];
    {{{ gpu.makeCheckDefined('bufferWrapper') }}}

    if (!bufferWrapper.onUnmap) {
      // Already unmapped
      return;
    }

    for (var i = 0; i < bufferWrapper.onUnmap.length; ++i) {
      bufferWrapper.onUnmap[i]();
    }
    bufferWrapper.onUnmap = undefined;

    bufferWrapper.object.unmap();
  },

  // wgpuTexture

  wgpuTextureGetDepthOrArrayLayers: (textureId) => {
    var texture = WebGPU.mgrTexture.get(textureId);
    return texture.depthOrArrayLayers;
  },

  wgpuTextureGetDimension: (textureId) => {
    var texture = WebGPU.mgrTexture.get(textureId);
    return WebGPU.TextureDimension.indexOf(texture.dimension);
  },

  wgpuTextureGetFormat: (textureId) => {
    var texture = WebGPU.mgrTexture.get(textureId);
    // Should return the enum integer instead of string.
    return WebGPU.TextureFormat.indexOf(texture.format);
  },

  wgpuTextureGetHeight: (textureId) => {
    var texture = WebGPU.mgrTexture.get(textureId);
    return texture.height;
  },

  wgpuTextureGetMipLevelCount: (textureId) => {
    var texture = WebGPU.mgrTexture.get(textureId);
    return texture.mipLevelCount;
  },

  wgpuTextureGetSampleCount: (textureId) => {
    var texture = WebGPU.mgrTexture.get(textureId);
    return texture.sampleCount;
  },

  wgpuTextureGetUsage: (textureId) => {
    var texture = WebGPU.mgrTexture.get(textureId);
    return texture.usage;
  },

  wgpuTextureGetWidth: (textureId) => {
    var texture = WebGPU.mgrTexture.get(textureId);
    return texture.width;
  },

  wgpuTextureSetLabel: (textureId, labelPtr) => {
    var texture = WebGPU.mgrTexture.get(textureId);
    texture.label = UTF8ToString(labelPtr);
  },

  wgpuTextureCreateView: (textureId, descriptor) => {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      var mipLevelCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.mipLevelCount) }}};
      var arrayLayerCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.arrayLayerCount) }}};
      desc = {
        "format": WebGPU.TextureFormat[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.format) }}}],
        "dimension": WebGPU.TextureViewDimension[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.dimension) }}}],
        "baseMipLevel": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.baseMipLevel) }}},
        "mipLevelCount": mipLevelCount === {{{ gpu.MIP_LEVEL_COUNT_UNDEFINED }}} ? undefined : mipLevelCount,
        "baseArrayLayer": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.baseArrayLayer) }}},
        "arrayLayerCount": arrayLayerCount === {{{ gpu.ARRAY_LAYER_COUNT_UNDEFINED }}} ? undefined : arrayLayerCount,
        "aspect": WebGPU.TextureAspect[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.aspect) }}}],
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }

    var texture = WebGPU.mgrTexture.get(textureId);
    return WebGPU.mgrTextureView.create(texture.createView(desc));
  },

  // wgpuTextureView

  wgpuTextureViewSetLabel: (textureViewId, labelPtr) => {
    var textureView = WebGPU.mgrTextureView.get(textureViewId);
    textureView.label = UTF8ToString(labelPtr);
  },

  // wgpuComputePass

  wgpuComputePassEncoderSetBindGroup: (passId, groupIndex, groupId, dynamicOffsetCount, dynamicOffsetsPtr) => {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    var group = WebGPU.mgrBindGroup.get(groupId);
    if (dynamicOffsetCount == 0) {
      pass.setBindGroup(groupIndex, group);
    } else {
      var offsets = [];
      for (var i = 0; i < dynamicOffsetCount; i++, dynamicOffsetsPtr += 4) {
        offsets.push({{{ gpu.makeGetU32('dynamicOffsetsPtr', 0) }}});
      }
      pass.setBindGroup(groupIndex, group, offsets);
    }
  },
  wgpuComputePassEncoderSetLabel: (passId, labelPtr) => {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    pass.label = UTF8ToString(labelPtr);
  },
  wgpuComputePassEncoderSetPipeline: (passId, pipelineId) => {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    var pipeline = WebGPU.mgrComputePipeline.get(pipelineId);
    pass.setPipeline(pipeline);
  },

  wgpuComputePassEncoderDispatchWorkgroups: (passId, x, y, z) => {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    pass.dispatchWorkgroups(x, y, z);
  },
  wgpuComputePassEncoderDispatchWorkgroupsIndirect: (passId, indirectBufferId, indirectOffset) => {
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    pass.dispatchWorkgroupsIndirect(indirectBuffer, indirectOffset);
  },

  wgpuComputePassEncoderWriteTimestamp: (encoderId, querySetId, queryIndex) => {
    var pass = WebGPU.mgrComputePassEncoder.get(encoderId);
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    pass.writeTimestamp(querySet, queryIndex);
  },

  wgpuComputePassEncoderPushDebugGroup: (encoderId, groupLabelPtr) => {
    var encoder = WebGPU.mgrComputePassEncoder.get(encoderId);
    encoder.pushDebugGroup(UTF8ToString(groupLabelPtr));
  },
  wgpuComputePassEncoderPopDebugGroup: (encoderId) => {
    var encoder = WebGPU.mgrComputePassEncoder.get(encoderId);
    encoder.popDebugGroup();
  },
  wgpuComputePassEncoderInsertDebugMarker: (encoderId, markerLabelPtr) => {
    var encoder = WebGPU.mgrComputePassEncoder.get(encoderId);
    encoder.insertDebugMarker(UTF8ToString(markerLabelPtr));
  },

  wgpuComputePassEncoderEnd: (passId) => {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    pass.end();
  },

  // wgpuRenderPass

  wgpuRenderPassEncoderSetLabel: (passId, labelPtr) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass.label = UTF8ToString(labelPtr);
  },

  wgpuRenderPassEncoderSetBindGroup: (passId, groupIndex, groupId, dynamicOffsetCount, dynamicOffsetsPtr) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var group = WebGPU.mgrBindGroup.get(groupId);
    if (dynamicOffsetCount == 0) {
      pass.setBindGroup(groupIndex, group);
    } else {
      var offsets = [];
      for (var i = 0; i < dynamicOffsetCount; i++, dynamicOffsetsPtr += 4) {
        offsets.push({{{ gpu.makeGetU32('dynamicOffsetsPtr', 0) }}});
      }
      pass.setBindGroup(groupIndex, group, offsets);
    }
  },
  wgpuRenderPassEncoderSetBlendConstant: (passId, colorPtr) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var color = WebGPU.makeColor(colorPtr);
    pass.setBlendConstant(color);
  },
  wgpuRenderPassEncoderSetIndexBuffer: (passId, bufferId, format, offset, size) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setIndexBuffer(buffer, WebGPU.IndexFormat[format], offset, size);
  },
  wgpuRenderPassEncoderSetPipeline: (passId, pipelineId) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var pipeline = WebGPU.mgrRenderPipeline.get(pipelineId);
    pass.setPipeline(pipeline);
  },
  wgpuRenderPassEncoderSetScissorRect: (passId, x, y, w, h) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass.setScissorRect(x, y, w, h);
  },
  wgpuRenderPassEncoderSetViewport: (passId, x, y, w, h, minDepth, maxDepth) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass.setViewport(x, y, w, h, minDepth, maxDepth);
  },
  wgpuRenderPassEncoderSetStencilReference: (passId, reference) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass.setStencilReference(reference);
  },
  wgpuRenderPassEncoderSetVertexBuffer: (passId, slot, bufferId, offset, size) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setVertexBuffer(slot, buffer, offset, size);
  },

  wgpuRenderPassEncoderDraw: (passId, vertexCount, instanceCount, firstVertex, firstInstance) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass.draw(vertexCount, instanceCount, firstVertex, firstInstance);
  },
  wgpuRenderPassEncoderDrawIndexed: (passId, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass.drawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
  },
  wgpuRenderPassEncoderDrawIndirect: (passId, indirectBufferId, indirectOffset) => {
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass.drawIndirect(indirectBuffer, indirectOffset);
  },
  wgpuRenderPassEncoderDrawIndexedIndirect: (passId, indirectBufferId, indirectOffset) => {
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass.drawIndexedIndirect(indirectBuffer, indirectOffset);
  },

  wgpuRenderPassEncoderExecuteBundles: (passId, count, bundlesPtr) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);

#if ASSERTIONS
    assert(bundlesPtr % 4 === 0);
#endif

    var bundles = Array.from({{{ makeHEAPView(`${POINTER_BITS}`, 'bundlesPtr', `bundlesPtr + count * ${POINTER_SIZE}`) }}},
      (id) => WebGPU.mgrRenderBundle.get(id));
    pass.executeBundles(bundles);
  },

  wgpuRenderPassEncoderBeginOcclusionQuery: (passId, queryIndex) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass.beginOcclusionQuery(queryIndex);
  },
  wgpuRenderPassEncoderEndOcclusionQuery: (passId) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass.endOcclusionQuery();
  },

  wgpuRenderPassEncoderWriteTimestamp: (encoderId, querySetId, queryIndex) => {
    var pass = WebGPU.mgrRenderPassEncoder.get(encoderId);
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    pass.writeTimestamp(querySet, queryIndex);
  },

  wgpuRenderPassEncoderPushDebugGroup: (encoderId, groupLabelPtr) => {
    var encoder = WebGPU.mgrRenderPassEncoder.get(encoderId);
    encoder.pushDebugGroup(UTF8ToString(groupLabelPtr));
  },
  wgpuRenderPassEncoderPopDebugGroup: (encoderId) => {
    var encoder = WebGPU.mgrRenderPassEncoder.get(encoderId);
    encoder.popDebugGroup();
  },
  wgpuRenderPassEncoderInsertDebugMarker: (encoderId, markerLabelPtr) => {
    var encoder = WebGPU.mgrRenderPassEncoder.get(encoderId);
    encoder.insertDebugMarker(UTF8ToString(markerLabelPtr));
  },
  wgpuRenderPassEncoderEnd: (encoderId) => {
    var encoder = WebGPU.mgrRenderPassEncoder.get(encoderId);
    encoder.end();
  },

  // Render bundle

  wgpuRenderBundleSetLabel: (bundleId, labelPtr) => {
    var bundle = WebGPU.mgrRenderBundle.get(bundleId);
    bundle.label = UTF8ToString(labelPtr);
  },

  // Render bundle encoder

  wgpuRenderBundleEncoderSetLabel: (bundleId, labelPtr) => {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass.label = UTF8ToString(labelPtr);
  },

  wgpuRenderBundleEncoderSetBindGroup: (bundleId, groupIndex, groupId, dynamicOffsetCount, dynamicOffsetsPtr) => {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var group = WebGPU.mgrBindGroup.get(groupId);
    if (dynamicOffsetCount == 0) {
      pass.setBindGroup(groupIndex, group);
    } else {
      var offsets = [];
      for (var i = 0; i < dynamicOffsetCount; i++, dynamicOffsetsPtr += 4) {
        offsets.push({{{ gpu.makeGetU32('dynamicOffsetsPtr', 0) }}});
      }
      pass.setBindGroup(groupIndex, group, offsets);
    }
  },
  wgpuRenderBundleEncoderSetIndexBuffer: (bundleId, bufferId, format, offset, size) => {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setIndexBuffer(buffer, WebGPU.IndexFormat[format], offset, size);
  },
  wgpuRenderBundleEncoderSetPipeline: (bundleId, pipelineId) => {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var pipeline = WebGPU.mgrRenderPipeline.get(pipelineId);
    pass.setPipeline(pipeline);
  },
  wgpuRenderBundleEncoderSetVertexBuffer: (bundleId, slot, bufferId, offset, size) => {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setVertexBuffer(slot, buffer, offset, size);
  },

  wgpuRenderBundleEncoderDraw: (bundleId, vertexCount, instanceCount, firstVertex, firstInstance) => {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass.draw(vertexCount, instanceCount, firstVertex, firstInstance);
  },
  wgpuRenderBundleEncoderDrawIndexed: (bundleId, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) => {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass.drawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
  },
  wgpuRenderBundleEncoderDrawIndirect: (bundleId, indirectBufferId, indirectOffset) => {
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass.drawIndirect(indirectBuffer, indirectOffset);
  },
  wgpuRenderBundleEncoderDrawIndexedIndirect: (bundleId, indirectBufferId, indirectOffset) => {
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass.drawIndexedIndirect(indirectBuffer, indirectOffset);
  },

  wgpuRenderBundleEncoderPushDebugGroup: (encoderId, groupLabelPtr) => {
    var encoder = WebGPU.mgrRenderBundleEncoder.get(encoderId);
    encoder.pushDebugGroup(UTF8ToString(groupLabelPtr));
  },
  wgpuRenderBundleEncoderPopDebugGroup: (encoderId) => {
    var encoder = WebGPU.mgrRenderBundleEncoder.get(encoderId);
    encoder.popDebugGroup();
  },
  wgpuRenderBundleEncoderInsertDebugMarker: (encoderId, markerLabelPtr) => {
    var encoder = WebGPU.mgrRenderBundleEncoder.get(encoderId);
    encoder.insertDebugMarker(UTF8ToString(markerLabelPtr));
  },

  wgpuRenderBundleEncoderFinish: (bundleId, descriptor) => {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {};
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderBundleDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }
    var encoder = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    return WebGPU.mgrRenderBundle.create(encoder.finish(desc));
  },

  // Instance

  wgpuInstanceCreateSurface__deps: ['$findCanvasEventTarget'],
  wgpuInstanceCreateSurface: (instanceId, descriptor) => {
    {{{ gpu.makeCheck('descriptor') }}}
    {{{ gpu.makeCheck('instanceId === 1, "WGPUInstance must be created by wgpuCreateInstance"') }}}
    var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSurfaceDescriptor.nextInChain, '*') }}};
#if ASSERTIONS
    assert(nextInChainPtr !== 0);
    assert({{{ gpu.SType.SurfaceDescriptorFromCanvasHTMLSelector }}} ===
      {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}});
#endif
    var descriptorFromCanvasHTMLSelector = nextInChainPtr;

    {{{ gpu.makeCheckDescriptor('descriptorFromCanvasHTMLSelector') }}}
    var selectorPtr = {{{ makeGetValue('descriptorFromCanvasHTMLSelector', C_STRUCTS.WGPUSurfaceDescriptorFromCanvasHTMLSelector.selector, '*') }}};
    {{{ gpu.makeCheck('selectorPtr') }}}
    var canvas = findCanvasEventTarget(selectorPtr);
#if OFFSCREENCANVAS_SUPPORT
    if (canvas.offscreenCanvas) canvas = canvas.offscreenCanvas;
#endif
    var context = canvas.getContext('webgpu');
#if ASSERTIONS
    assert(context);
#endif
    if (!context) return 0;

    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSurfaceDescriptor.label, '*') }}};
    if (labelPtr) context.surfaceLabelWebGPU = UTF8ToString(labelPtr);

    return WebGPU.mgrSurface.create(context);
  },

  wgpuInstanceHasWGSLLanguageFeature: (instance, featureEnumValue) => {
    if (!('wgslLanguageFeatures' in navigator["gpu"])) {
      return false;
    }
    return navigator["gpu"]["wgslLanguageFeatures"].has(WebGPU.WGSLFeatureName[featureEnumValue]);
  },

  wgpuInstanceProcessEvents: (instance) => {
    // TODO: This could probably be emulated with ASYNCIFY.
#if ASSERTIONS
    abort('wgpuInstanceProcessEvents is unsupported (use requestAnimationFrame via html5.h instead)');
#endif
  },

  wgpuInstanceRequestAdapter__deps: ['$callUserCallback', '$stringToUTF8OnStack'],
  wgpuInstanceRequestAdapter: (instanceId, options, callback, userdata) => {
    {{{ gpu.makeCheck('instanceId === 1, "WGPUInstance must be created by wgpuCreateInstance"') }}}

    var opts;
    if (options) {
      {{{ gpu.makeCheckDescriptor('options') }}}
      opts = {
        "powerPreference": WebGPU.PowerPreference[
          {{{ gpu.makeGetU32('options', C_STRUCTS.WGPURequestAdapterOptions.powerPreference) }}}],
        "forceFallbackAdapter":
          {{{ gpu.makeGetBool('options', C_STRUCTS.WGPURequestAdapterOptions.forceFallbackAdapter) }}},
      };
    }

    if (!('gpu' in navigator)) {
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack('WebGPU not available on this browser (navigator.gpu is not available)');
      {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.RequestAdapterStatus.Unavailable }}}, 0, messagePtr, userdata);
      stackRestore(sp);
      return;
    }

    {{{ runtimeKeepalivePush() }}}
    navigator["gpu"]["requestAdapter"](opts).then((adapter) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        if (adapter) {
          var adapterId = WebGPU.mgrAdapter.create(adapter);
          {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.RequestAdapterStatus.Success }}}, adapterId, 0, userdata);
        } else {
          var sp = stackSave();
          var messagePtr = stringToUTF8OnStack('WebGPU not available on this system (requestAdapter returned null)');
          {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.RequestAdapterStatus.Unavailable }}}, 0, messagePtr, userdata);
          stackRestore(sp);
        }
      });
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        var sp = stackSave();
        var messagePtr = stringToUTF8OnStack(ex.message);
        {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.RequestAdapterStatus.Error }}}, 0, messagePtr, userdata);
        stackRestore(sp);
      });
    });
  },

  // WGPUAdapter

  wgpuAdapterEnumerateFeatures: (adapterId, featuresOutPtr) => {
    var adapter = WebGPU.mgrAdapter.get(adapterId);
    if (featuresOutPtr !== 0) {
      var offset = 0;
      adapter.features.forEach(feature => {
        var featureEnumValue = WebGPU.FeatureNameString2Enum[feature];
        {{{ makeSetValue('featuresOutPtr', 'offset', 'featureEnumValue', 'i32') }}};
        offset += 4;
      });
    }
    return adapter.features.size;
  },

  wgpuAdapterGetInfo__deps: ['$stringToNewUTF8'],
  wgpuAdapterGetInfo: (adapterId, info) => {
    var adapter = WebGPU.mgrAdapter.get(adapterId);
    {{{ gpu.makeCheckDescriptor('info') }}}

    var vendorPtr = stringToNewUTF8(adapter.info.vendor);
    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.vendor, 'vendorPtr', '*') }}};
    var architecturePtr = stringToNewUTF8(adapter.info.architecture);
    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.architecture, 'architecturePtr', '*') }}};
    var devicePtr = stringToNewUTF8(adapter.info.device);
    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.device, 'devicePtr', '*') }}};
    var descriptionPtr = stringToNewUTF8(adapter.info.description);
    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.description, 'descriptionPtr', '*') }}};
    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.backendType, gpu.BackendType.WebGPU, 'i32') }}};
    var adapterType = adapter.isFallbackAdapter ? {{{ gpu.AdapterType.CPU }}} : {{{ gpu.AdapterType.Unknown }}};
    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.adapterType, 'adapterType', 'i32') }}};
    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.vendorID, '0', 'i32') }}};
    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.deviceID, '0', 'i32') }}};
  },

  wgpuAdapterGetProperties__deps: ['$warnOnce'],
  wgpuAdapterGetProperties: (adapterId, properties) => {
    warnOnce('wgpuAdapterGetProperties is deprecated, use wgpuAdapterGetInfo instead');

    {{{ gpu.makeCheckDescriptor('properties') }}}
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.vendorID, '0', 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.vendorName, '0', 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.architecture, '0', 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.deviceID, '0', 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.name, '0', 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.driverDescription, '0', 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.adapterType, gpu.AdapterType.Unknown, 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.backendType, gpu.BackendType.WebGPU, 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.compatibilityMode, '0', 'i32') }}};
  },

  wgpuAdapterGetLimits: (adapterId, limitsOutPtr) => {
    var adapter = WebGPU.mgrAdapter.get(adapterId);
    WebGPU.fillLimitStruct(adapter.limits, limitsOutPtr);
    return 1;
  },

  wgpuAdapterHasFeature: (adapterId, featureEnumValue) => {
    var adapter = WebGPU.mgrAdapter.get(adapterId);
    return adapter.features.has(WebGPU.FeatureName[featureEnumValue]);
  },

  wgpuAdapterRequestDevice__deps: ['$callUserCallback', '$stringToUTF8OnStack'],
  wgpuAdapterRequestDevice: (adapterId, descriptor, callback, userdata) => {
    var adapter = WebGPU.mgrAdapter.get(adapterId);

    var desc = {};
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      var requiredFeatureCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredFeatureCount) }}};
      if (requiredFeatureCount) {
        var requiredFeaturesPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredFeatures, '*') }}};
        desc["requiredFeatures"] = Array.from({{{ makeHEAPView('32', 'requiredFeaturesPtr', `requiredFeaturesPtr + requiredFeatureCount * ${POINTER_SIZE}`) }}},
          (feature) => WebGPU.FeatureName[feature]);
      }
      var requiredLimitsPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredLimits, '*') }}};
      if (requiredLimitsPtr) {
        {{{ gpu.makeCheckDescriptor('requiredLimitsPtr') }}}
        var limitsPtr = requiredLimitsPtr + {{{ C_STRUCTS.WGPURequiredLimits.limits }}};
        var requiredLimits = {};
        function setLimitU32IfDefined(name, limitOffset) {
          var ptr = limitsPtr + limitOffset;
          var value = {{{ gpu.makeGetU32('ptr', 0) }}};
          if (value != {{{ gpu.LIMIT_U32_UNDEFINED }}}) {
            requiredLimits[name] = value;
          }
        }
        function setLimitU64IfDefined(name, limitOffset) {
          var ptr = limitsPtr + limitOffset;
          // Handle WGPU_LIMIT_U64_UNDEFINED.
          var limitPart1 = {{{ gpu.makeGetU32('ptr', 0) }}};
          var limitPart2 = {{{ gpu.makeGetU32('ptr', 4) }}};
          if (limitPart1 != 0xFFFFFFFF || limitPart2 != 0xFFFFFFFF) {
            requiredLimits[name] = {{{ gpu.makeGetU64('ptr', 0) }}}
          }
        }

        setLimitU32IfDefined("maxTextureDimension1D", {{{ C_STRUCTS.WGPULimits.maxTextureDimension1D }}});
        setLimitU32IfDefined("maxTextureDimension2D", {{{ C_STRUCTS.WGPULimits.maxTextureDimension2D }}});
        setLimitU32IfDefined("maxTextureDimension3D", {{{ C_STRUCTS.WGPULimits.maxTextureDimension3D }}});
        setLimitU32IfDefined("maxTextureArrayLayers", {{{ C_STRUCTS.WGPULimits.maxTextureArrayLayers }}});
        setLimitU32IfDefined("maxBindGroups", {{{ C_STRUCTS.WGPULimits.maxBindGroups }}});
        setLimitU32IfDefined('maxBindGroupsPlusVertexBuffers', {{{ C_STRUCTS.WGPULimits.maxBindGroupsPlusVertexBuffers }}});
        setLimitU32IfDefined("maxDynamicUniformBuffersPerPipelineLayout", {{{ C_STRUCTS.WGPULimits.maxDynamicUniformBuffersPerPipelineLayout }}});
        setLimitU32IfDefined("maxDynamicStorageBuffersPerPipelineLayout", {{{ C_STRUCTS.WGPULimits.maxDynamicStorageBuffersPerPipelineLayout }}});
        setLimitU32IfDefined("maxSampledTexturesPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxSampledTexturesPerShaderStage }}});
        setLimitU32IfDefined("maxSamplersPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxSamplersPerShaderStage }}});
        setLimitU32IfDefined("maxStorageBuffersPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxStorageBuffersPerShaderStage }}});
        setLimitU32IfDefined("maxStorageTexturesPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxStorageTexturesPerShaderStage }}});
        setLimitU32IfDefined("maxUniformBuffersPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxUniformBuffersPerShaderStage }}});
        setLimitU32IfDefined("minUniformBufferOffsetAlignment", {{{ C_STRUCTS.WGPULimits.minUniformBufferOffsetAlignment }}});
        setLimitU32IfDefined("minStorageBufferOffsetAlignment", {{{ C_STRUCTS.WGPULimits.minStorageBufferOffsetAlignment }}});
        setLimitU64IfDefined("maxUniformBufferBindingSize", {{{ C_STRUCTS.WGPULimits.maxUniformBufferBindingSize }}});
        setLimitU64IfDefined("maxStorageBufferBindingSize", {{{ C_STRUCTS.WGPULimits.maxStorageBufferBindingSize }}});
        setLimitU32IfDefined("maxVertexBuffers", {{{ C_STRUCTS.WGPULimits.maxVertexBuffers }}});
        setLimitU64IfDefined("maxBufferSize", {{{ C_STRUCTS.WGPULimits.maxBufferSize }}});
        setLimitU32IfDefined("maxVertexAttributes", {{{ C_STRUCTS.WGPULimits.maxVertexAttributes }}});
        setLimitU32IfDefined("maxVertexBufferArrayStride", {{{ C_STRUCTS.WGPULimits.maxVertexBufferArrayStride }}});
        setLimitU32IfDefined("maxInterStageShaderComponents", {{{ C_STRUCTS.WGPULimits.maxInterStageShaderComponents }}});
        setLimitU32IfDefined("maxInterStageShaderVariables", {{{ C_STRUCTS.WGPULimits.maxInterStageShaderVariables }}});
        setLimitU32IfDefined("maxColorAttachments", {{{ C_STRUCTS.WGPULimits.maxColorAttachments }}});
        setLimitU32IfDefined("maxColorAttachmentBytesPerSample", {{{ C_STRUCTS.WGPULimits.maxColorAttachmentBytesPerSample }}});
        setLimitU32IfDefined("maxComputeWorkgroupStorageSize", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupStorageSize }}});
        setLimitU32IfDefined("maxComputeInvocationsPerWorkgroup", {{{ C_STRUCTS.WGPULimits.maxComputeInvocationsPerWorkgroup }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeX", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeX }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeY", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeY }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeZ", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeZ }}});
        setLimitU32IfDefined("maxComputeWorkgroupsPerDimension", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupsPerDimension }}});
        desc["requiredLimits"] = requiredLimits;
      }

      var defaultQueuePtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.defaultQueue, '*') }}};
      if (defaultQueuePtr) {
        var defaultQueueDesc = {};
        var labelPtr = {{{ makeGetValue('defaultQueuePtr', C_STRUCTS.WGPUQueueDescriptor.label, '*') }}};
        if (labelPtr) defaultQueueDesc["label"] = UTF8ToString(labelPtr);
        desc["defaultQueue"] = defaultQueueDesc;
      }

      var deviceLostCallbackPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.deviceLostCallbackInfo + C_STRUCTS.WGPUDeviceLostCallbackInfo.callback, '*') }}};
      var deviceLostUserdataPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.deviceLostCallbackInfo + C_STRUCTS.WGPUDeviceLostCallbackInfo.userdata, '*') }}};

      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }

    {{{ runtimeKeepalivePush() }}}
    adapter.requestDevice(desc).then((device) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        var deviceWrapper = { queueId: WebGPU.mgrQueue.create(device.queue) };
        var deviceId = WebGPU.mgrDevice.create(device, deviceWrapper);
        if (deviceLostCallbackPtr) {
          device.lost.then((info) => {
            callUserCallback(() => WebGPU.errorCallback(deviceLostCallbackPtr,
              WebGPU.Int_DeviceLostReason[info.reason], info.message, deviceLostUserdataPtr));
          });
        }
        {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.RequestDeviceStatus.Success }}}, deviceId, 0, userdata);
      });
    }, function(ex) {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        var sp = stackSave();
        var messagePtr = stringToUTF8OnStack(ex.message);
        {{{ makeDynCall('vippp', 'callback') }}}({{{ gpu.RequestDeviceStatus.Error }}}, 0, messagePtr, userdata);
        stackRestore(sp);
      });
    });
  },

  // WGPUAdapterProperties

  wgpuAdapterPropertiesFreeMembers: (value) => {
    // wgpuAdapterGetProperties doesn't currently allocate anything.
  },

  // WGPUSampler

  wgpuSamplerSetLabel: (samplerId, labelPtr) => {
    var sampler = WebGPU.mgrSampler.get(samplerId);
    sampler.label = UTF8ToString(labelPtr);
  },

  // WGPUSurface

  wgpuSurfaceConfigure: (surfaceId, config) => {
    {{{ gpu.makeCheckDescriptor('config') }}}
    var deviceId = {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.device, '*') }}};
    var context = WebGPU.mgrSurface.get(surfaceId);

#if ASSERTIONS
    var viewFormatCount = {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.viewFormatCount) }}};
    var viewFormats = {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.viewFormats, '*') }}};
    assert(viewFormatCount === 0 && viewFormats === 0, "TODO: Support viewFormats.");
    var alphaMode = {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.alphaMode) }}};
    assert(alphaMode === {{{ gpu.CompositeAlphaMode.Auto }}} ||
      alphaMode === {{{ gpu.CompositeAlphaMode.Opaque }}},
      "TODO: Support WGPUCompositeAlphaMode_Premultiplied.");
    assert({{{ gpu.PresentMode.Fifo }}} ===
      {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.presentMode) }}});
#endif

    var canvasSize = [
      {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.width) }}},
      {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.height) }}}
    ];

    if (canvasSize[0] !== 0) {
      context["canvas"]["width"] = canvasSize[0];
    }

    if (canvasSize[1] !== 0) {
      context["canvas"]["height"] = canvasSize[1];
    }

    var configuration = {
      "device": WebGPU.mgrDevice.get(deviceId),
      "format": WebGPU.TextureFormat[
        {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.format) }}}],
      "usage": {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.usage) }}},
      "alphaMode": "opaque",
    };
    context.configure(configuration);
  },

  wgpuSurfaceGetCurrentTexture: (surfaceId, surfaceTexturePtr) => {
    {{{ gpu.makeCheck('surfaceTexturePtr') }}}
    var context = WebGPU.mgrSurface.get(surfaceId);

    try {
      var texture = WebGPU.mgrTexture.create(context.getCurrentTexture());
      {{{ makeSetValue('surfaceTexturePtr', C_STRUCTS.WGPUSurfaceTexture.texture, 'texture', '*') }}};
      {{{ makeSetValue('surfaceTexturePtr', C_STRUCTS.WGPUSurfaceTexture.suboptimal, '0', 'i32') }}};
      {{{ makeSetValue('surfaceTexturePtr', C_STRUCTS.WGPUSurfaceTexture.status,
        gpu.SurfaceGetCurrentTextureStatus.Success, 'i32') }}};
    } catch (ex) {
#if ASSERTIONS
      err(`wgpuSurfaceGetCurrentTexture() failed: ${ex}`);
#endif
      {{{ makeSetValue('surfaceTexturePtr', C_STRUCTS.WGPUSurfaceTexture.texture, '0', '*') }}};
      {{{ makeSetValue('surfaceTexturePtr', C_STRUCTS.WGPUSurfaceTexture.suboptimal, '0', 'i32') }}};
      // TODO(https://github.com/webgpu-native/webgpu-headers/issues/291): What should the status be here?
      {{{ makeSetValue('surfaceTexturePtr', C_STRUCTS.WGPUSurfaceTexture.status,
        gpu.SurfaceGetCurrentTextureStatus.DeviceLost, 'i32') }}};
    }
  },

  wgpuSurfaceGetPreferredFormat: (surfaceId, adapterId) => {
    var format = navigator["gpu"]["getPreferredCanvasFormat"]();
    return WebGPU.Int_PreferredFormat[format];
  },

  wgpuSurfacePresent: (surfaceId) => {
    // TODO: This could probably be emulated with ASYNCIFY.
    abort('wgpuSurfacePresent is unsupported (use requestAnimationFrame via html5.h instead)');
  },

  wgpuSurfaceUnconfigure: (surfaceId) => {
    var context = WebGPU.mgrSurface.get(surfaceId);
    context.unconfigure();
  },

  // WGPUSwapChain

  wgpuDeviceCreateSwapChain: (deviceId, surfaceId, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}
    var device = WebGPU.mgrDevice.get(deviceId);
    var context = WebGPU.mgrSurface.get(surfaceId);

#if ASSERTIONS
    assert({{{ gpu.PresentMode.Fifo }}} ===
      {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.presentMode) }}});
#endif

    var canvasSize = [
      {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.width) }}},
      {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.height) }}}
    ];

    if (canvasSize[0] !== 0) {
      context["canvas"]["width"] = canvasSize[0];
    }

    if (canvasSize[1] !== 0) {
      context["canvas"]["height"] = canvasSize[1];
    }

    var configuration = {
      "device": device,
      "format": WebGPU.TextureFormat[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.format) }}}],
      "usage": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.usage) }}},
      "alphaMode": "opaque",
    };
    context.configure(configuration);

    return WebGPU.mgrSwapChain.create(context);
  },

  wgpuSwapChainGetCurrentTexture: (swapChainId) => {
    var context = WebGPU.mgrSwapChain.get(swapChainId);
    return WebGPU.mgrTexture.create(context.getCurrentTexture());
  },
  wgpuSwapChainGetCurrentTextureView: (swapChainId) => {
    var context = WebGPU.mgrSwapChain.get(swapChainId);
    return WebGPU.mgrTextureView.create(context.getCurrentTexture().createView());
  },
  wgpuSwapChainPresent: (swapChainId) => {
    // TODO: This could probably be emulated with ASYNCIFY.
    abort('wgpuSwapChainPresent is unsupported (use requestAnimationFrame via html5.h instead)');
  },
};

// Inverted index used by EnumerateFeatures/HasFeature
LibraryWebGPU.$WebGPU.FeatureNameString2Enum = {};
for (var value in LibraryWebGPU.$WebGPU.FeatureName) {
  LibraryWebGPU.$WebGPU.FeatureNameString2Enum[LibraryWebGPU.$WebGPU.FeatureName[value]] = value;
}

for (const key of Object.keys(LibraryWebGPU)) {
  if (typeof LibraryWebGPU[key] === 'function') {
    LibraryWebGPU[key + '__i53abi'] = true;
  }
}

autoAddDeps(LibraryWebGPU, '$WebGPU');
mergeInto(LibraryManager.library, LibraryWebGPU);
