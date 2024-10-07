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
      // ShaderSourceSPIRV), there is no .nextInChain pointer, but
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
  $WebGPU__deps: ['$stackSave', '$stackRestore', '$stringToUTF8OnStack'],
  $WebGPU: {

    // Object management is consolidated into a single table that doesn't care
    // about object type, and is keyed on the pointer address to a refcount.
    // Note that most objects are directly stored in the table with the
    // exception of Buffers which are stored within a wrapper. The wrapper is
    // currently necessary to handle mapping and unmapping of the Buffer.
    _table: [],
    _tableGet: (ptr) => {
      if (!ptr) return undefined;
      return WebGPU._table[ptr];
    },
    _tableInsert: (ptr, value) => {
      WebGPU._table[ptr] = value;
    },

    // Future to promise management, and temporary list allocated up-front for
    // WaitAny implementation on the promises. Note that all FutureIDs
    // (uint64_t) are passed either as a low and high value or by pointer
    // because they need to be passed back and forth between JS and C++, and JS
    // is currently unable to pass a value to a C++ function as a uint64_t.
    // This might be possible with -sWASM_BIGINT, but I was unable to get that
    // to work properly at the time of writing.
    _futures: [],
    _futureInsert: (futureIdL, futureIdH, promise) => {
#if ASYNCIFY
      var futureId = futureIdH * 0x100000000 + futureIdL;
      WebGPU._futures[futureId] =
        new Promise((resolve) => promise.finally(() => resolve(futureId)));
#endif
    },
    _waitAnyPromisesList: [],

    errorCallback: (callback, type, message, userdata) => {
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack(message);
      {{{ makeDynCall('vipp', 'callback') }}}(type, messagePtr, userdata);
      stackRestore(sp);
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
        "texture": WebGPU._tableGet(
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
      bufferCopyView["buffer"] = WebGPU._tableGet(
        {{{ makeGetValue('ptr', C_STRUCTS.WGPUImageCopyBuffer.buffer, '*') }}}).object;
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
      return WebGPU._tableGet(layoutPtr);
    },

    makeProgrammableStageDescriptor: (ptr) => {
      if (!ptr) return undefined;
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      var desc = {
        "module": WebGPU._tableGet(
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

      function makePrimitiveState(psPtr) {
        if (!psPtr) return undefined;
        {{{ gpu.makeCheckDescriptor('psPtr') }}}
        return {
          "topology": WebGPU.PrimitiveTopology[
            {{{ gpu.makeGetU32('psPtr', C_STRUCTS.WGPUPrimitiveState.topology) }}}],
          "stripIndexFormat": WebGPU.IndexFormat[
            {{{ gpu.makeGetU32('psPtr', C_STRUCTS.WGPUPrimitiveState.stripIndexFormat) }}}],
          "frontFace": WebGPU.FrontFace[
            {{{ gpu.makeGetU32('psPtr', C_STRUCTS.WGPUPrimitiveState.frontFace) }}}],
          "cullMode": WebGPU.CullMode[
            {{{ gpu.makeGetU32('psPtr', C_STRUCTS.WGPUPrimitiveState.cullMode) }}}],
          "unclippedDepth":
            {{{ gpu.makeGetBool('psPtr', C_STRUCTS.WGPUPrimitiveState.unclippedDepth) }}},
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
          "module": WebGPU._tableGet(
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
          "module": WebGPU._tableGet(
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

  // ----------------------------------------------------------------------------
  // Definitions for standalone JS emwgpu functions (callable from webgpu.cpp and
  //   library_html5_html.js)
  // ----------------------------------------------------------------------------

  emwgpuDelete: (id) => {
    delete WebGPU._table[id];
  },

  // Extra helper that allow for directly inserting Devices (and their
  // corresponding Queue) that is called from the HTML5 library since there
  // isn't access to the C++ in webgpu.cpp there.
  emwgpuTableInsertDevice__deps: ['emwgpuCreateDevice', 'emwgpuCreateQueue', 'wgpuCreateInstance'],
  emwgpuTableInsertDevice: (device) => {
    var instancePtr = _wgpuCreateInstance();
    var queuePtr = _emwgpuCreateQueue();
    WebGPU._tableInsert(queuePtr, device.queue);
    var devicePtr = _emwgpuCreateDevice(instancePtr, queuePtr);
    WebGPU._tableInsert(devicePtr, device);
    return { instancePtr, devicePtr };
  },

#if ASYNCIFY
  // Returns a FutureID that was resolved, or kNullFutureId if timed out.
  emwgpuWaitAny__async: true,
  emwgpuWaitAny: (futurePtr, futureCount, timeoutNSPtr) => {
    var promises = WebGPU._waitAnyPromisesList;
    if (timeoutNSPtr) {
      var timeoutMS = {{{ gpu.makeGetU64('timeoutNSPtr', 0) }}} / 1000000;
      promises.length = futureCount + 1;
      promise[futureCount] = new Promise((resolve) => setTimeout(resolve, timeoutMS, 0));
    } else {
      promises.length = futureCount;
    }

    for (var i = 0; i < futureCount; ++i) {
      // If any of the FutureIDs are not tracked, it means it must be done.
      var futureId = {{{ gpu.makeGetU64('(futurePtr + i * 8)', 0) }}};
      if (!(futureId in WebGPU._futures)) {
        return futureId;
      }
      promises[i] = WebGPU._futures[futureId];
    }

    var result = Asyncify.handleAsync(async () => {
      return await Promise.race(promises);
    });

    // Clean up internal futures state.
    delete WebGPU._futures[result];
    WebGPU._waitAnyPromisesList.length = 0;
    return result;
  },
#endif

  emwgpuGetPreferredFormat: () => {
    var format = navigator["gpu"]["getPreferredCanvasFormat"]();
    return WebGPU.Int_PreferredFormat[format];
  },

  // --------------------------------------------------------------------------
  // WebGPU function definitions, with methods organized by "class".
  //
  // Also note that the full set of functions declared in webgpu.h are only
  // partially implemeted here. The remaining ones are implemented via
  // webgpu.cpp.
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // Standalone (non-method) functions
  // --------------------------------------------------------------------------

  wgpuGetInstanceFeatures: (featuresPtr) => {
    abort('TODO: wgpuGetInstanceFeatures unimplemented');
    return 0;
  },

  wgpuGetProcAddress: (device, procName) => {
    abort('TODO(#11526): wgpuGetProcAddress unimplemented');
    return 0;
  },

  // --------------------------------------------------------------------------
  // Methods of Adapter
  // --------------------------------------------------------------------------

  wgpuAdapterEnumerateFeatures: (adapterPtr, featuresOutPtr) => {
    var adapter = WebGPU._tableGet(adapterPtr);
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
  wgpuAdapterGetInfo: (adapterPtr, info) => {
    var adapter = WebGPU._tableGet(adapterPtr);
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

  wgpuAdapterGetLimits: (adapterPtr, limitsOutPtr) => {
    var adapter = WebGPU._tableGet(adapterPtr);
    WebGPU.fillLimitStruct(adapter.limits, limitsOutPtr);
    return 1;
  },

  wgpuAdapterHasFeature: (adapterPtr, featureEnumValue) => {
    var adapter = WebGPU._tableGet(adapterPtr);
    return adapter.features.has(WebGPU.FeatureName[featureEnumValue]);
  },

  emwgpuAdapterRequestDevice__i53abi: false,
  emwgpuAdapterRequestDevice__deps: ['$stringToUTF8OnStack', 'emwgpuCreateQueue', 'emwgpuOnDeviceLostCompleted', 'emwgpuOnRequestDeviceCompleted', 'emwgpuOnUncapturedError'],
  emwgpuAdapterRequestDevice: (
    adapterPtr,
    futureIdL, futureIdH,
    deviceLostFutureIdL, deviceLostFutureIdH,
    devicePtr, queuePtr, descriptor
  ) => {
    var adapter = WebGPU._tableGet(adapterPtr);

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

      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }

    {{{ runtimeKeepalivePush() }}}
    var hasDeviceLostFutureId = !!deviceLostFutureIdH || !!deviceLostFutureIdL;
    WebGPU._futureInsert(futureIdL, futureIdH, adapter.requestDevice(desc).then((device) => {
      {{{ runtimeKeepalivePop() }}}
      WebGPU._tableInsert(queuePtr, device.queue);
      WebGPU._tableInsert(devicePtr, device);

      // Set up device lost promise resolution.
      if (hasDeviceLostFutureId) {
        {{{ runtimeKeepalivePush() }}}
        WebGPU._futureInsert(deviceLostFutureIdL, deviceLostFutureIdH, device.lost.then((info) => {
          {{{ runtimeKeepalivePop() }}}
          // Unset the uncaptured error handler.
          device.onuncapturederror = (ev) => {};
          var sp = stackSave();
          var messagePtr = stringToUTF8OnStack(info.message);
          _emwgpuOnDeviceLostCompleted(deviceLostFutureIdL, deviceLostFutureIdH, WebGPU.Int_DeviceLostReason[info.reason], messagePtr);
          stackRestore(sp);
        }));
      }

      // Set up uncaptured error handlers.
#if ASSERTIONS
      assert(typeof GPUValidationError != 'undefined');
      assert(typeof GPUOutOfMemoryError != 'undefined');
      assert(typeof GPUInternalError != 'undefined');
#endif
      device.onuncapturederror = (ev) => {
          var type = {{{ gpu.ErrorType.Unknown }}};;
          if (ev.error instanceof GPUValidationError) type = {{{ gpu.ErrorType.Validation }}};
          else if (ev.error instanceof GPUOutOfMemoryError) type = {{{ gpu.ErrorType.OutOfMemory }}};
          else if (ev.error instanceof GPUInternalError) type = {{{ gpu.ErrorType.Internal }}};
          var sp = stackSave();
          var messagePtr = stringToUTF8OnStack(ev.error.message);
          _emwgpuOnUncapturedError(devicePtr, type, messagePtr);
          stackRestore(sp);
      };

      _emwgpuOnRequestDeviceCompleted(futureIdL, futureIdH, {{{ gpu.RequestDeviceStatus.Success }}}, devicePtr, 0);
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}}
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack(ex.message);
      _emwgpuOnRequestDeviceCompleted(futureIdL, futureIdH, {{{ gpu.RequestDeviceStatus.Error }}}, devicePtr, messagePtr);
      if (hasDeviceLostFutureId) {
        _emwgpuOnDeviceLostCompleted(deviceLostFutureIdL, deviceLostFutureIdH, {{{ gpu.DeviceLostReason.FailedCreation }}}, messagePtr);
      }
      stackRestore(sp);
    }));
  },

  // --------------------------------------------------------------------------
  // Methods of BindGroup
  // --------------------------------------------------------------------------

  wgpuBindGroupSetLabel: (bindGroupPtr, labelPtr) => {
    var bindGroup = WebGPU._tableGet(bindGroupPtr);
    bindGroup.label = UTF8ToString(labelPtr);
  },

  // --------------------------------------------------------------------------
  // Methods of BindGroupLayout
  // --------------------------------------------------------------------------

  wgpuBindGroupLayoutSetLabel: (bindGroupLayoutPtr, labelPtr) => {
    var bindGroupLayout = WebGPU._tableGet(bindGroupLayoutPtr);
    bindGroupLayout.label = UTF8ToString(labelPtr);
  },

  // --------------------------------------------------------------------------
  // Methods of Buffer
  // --------------------------------------------------------------------------

  wgpuBufferDestroy: (bufferPtr) => {
    var bufferWrapper = WebGPU._tableGet(bufferPtr);
    {{{ gpu.makeCheckDefined('bufferWrapper') }}}
    if (bufferWrapper.onUnmap) {
      for (var i = 0; i < bufferWrapper.onUnmap.length; ++i) {
        bufferWrapper.onUnmap[i]();
      }
      bufferWrapper.onUnmap = undefined;
    }

    bufferWrapper.object.destroy();
  },

  // In webgpu.h offset and size are passed in as size_t.
  // And library_webgpu assumes that size_t is always 32bit in emscripten.
  wgpuBufferGetConstMappedRange__deps: ['$warnOnce', 'memalign', 'free'],
  wgpuBufferGetConstMappedRange: (bufferPtr, offset, size) => {
    var bufferWrapper = WebGPU._tableGet(bufferPtr);
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

  wgpuBufferGetMapState: (bufferPtr) => {
    var buffer = WebGPU._tableGet(bufferPtr).object;
    return WebGPU.Int_BufferMapState[buffer.mapState];
  },

  // In webgpu.h offset and size are passed in as size_t.
  // And library_webgpu assumes that size_t is always 32bit in emscripten.
  wgpuBufferGetMappedRange__deps: ['$warnOnce', 'memalign', 'free'],
  wgpuBufferGetMappedRange: (bufferPtr, offset, size) => {
    var bufferWrapper = WebGPU._tableGet(bufferPtr);
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

  wgpuBufferGetSize: (bufferPtr) => {
    var buffer = WebGPU._tableGet(bufferPtr).object;
    // 64-bit
    return buffer.size;
  },

  wgpuBufferGetUsage: (bufferPtr) => {
    var buffer = WebGPU._tableGet(bufferPtr).object;
    return buffer.usage;
  },

  // In webgpu.h offset and size are passed in as size_t.
  // And library_webgpu assumes that size_t is always 32bit in emscripten.
  wgpuBufferMapAsync__deps: ['$callUserCallback'],
  wgpuBufferMapAsync: (bufferPtr, mode, offset, size, callback, userdata) => {
    var bufferWrapper = WebGPU._tableGet(bufferPtr);
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

  wgpuBufferSetLabel: (bufferPtr, labelPtr) => {
    var buffer = WebGPU._tableGet(bufferPtr).object;
    buffer.label = UTF8ToString(labelPtr);
  },

  wgpuBufferUnmap: (bufferPtr) => {
    var bufferWrapper = WebGPU._tableGet(bufferPtr);
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

  // --------------------------------------------------------------------------
  // Methods of CommandBuffer
  // --------------------------------------------------------------------------

  wgpuCommandBufferSetLabel: (commandBufferPtr, labelPtr) => {
    var commandBuffer = WebGPU._tableGet(commandBufferPtr);
    commandBuffer.label = UTF8ToString(labelPtr);
  },

  // --------------------------------------------------------------------------
  // Methods of CommandEncoder
  // --------------------------------------------------------------------------

  wgpuCommandEncoderBeginComputePass__deps: ['emwgpuCreateComputePassEncoder'],
  wgpuCommandEncoderBeginComputePass: (encoderPtr, descriptor) => {
    var desc;

    function makeComputePassTimestampWrites(twPtr) {
      if (twPtr === 0) return undefined;

      return {
        "querySet": WebGPU._tableGet(
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
    var commandEncoder = WebGPU._tableGet(encoderPtr);
    var ptr = _emwgpuCreateComputePassEncoder();
    WebGPU._tableInsert(ptr, commandEncoder.beginComputePass(desc));
    return ptr;
  },

  wgpuCommandEncoderBeginRenderPass__deps: ['emwgpuCreateRenderPassEncoder'],
  wgpuCommandEncoderBeginRenderPass: (encoderPtr, descriptor) => {
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
        "view": WebGPU._tableGet(viewPtr),
        "depthSlice": depthSlice,
        "resolveTarget": WebGPU._tableGet(
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
        "view": WebGPU._tableGet(
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
        "querySet": WebGPU._tableGet(
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
        assert(sType === {{{ gpu.SType.RenderPassMaxDrawCount }}});
        assert(0 === {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.next, '*') }}});
#endif
        var renderPassMaxDrawCount = nextInChainPtr;
        {{{ gpu.makeCheckDescriptor('renderPassMaxDrawCount') }}}
        maxDrawCount = {{{ gpu.makeGetU64('renderPassMaxDrawCount', C_STRUCTS.WGPURenderPassMaxDrawCount.maxDrawCount) }}};
      }

      var desc = {
        "label": undefined,
        "colorAttachments": makeColorAttachments(
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderPassDescriptor.colorAttachmentCount) }}},
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.colorAttachments, '*') }}}),
        "depthStencilAttachment": makeDepthStencilAttachment(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.depthStencilAttachment, '*') }}}),
        "occlusionQuerySet": WebGPU._tableGet(
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

    var commandEncoder = WebGPU._tableGet(encoderPtr);
    var ptr = _emwgpuCreateRenderPassEncoder();
    WebGPU._tableInsert(ptr, commandEncoder.beginRenderPass(desc));
    return ptr;
  },

  wgpuCommandEncoderClearBuffer: (encoderPtr, bufferPtr, offset, size) => {
    var commandEncoder = WebGPU._tableGet(encoderPtr);
    {{{ gpu.convertSentinelToUndefined('size') }}}

    var buffer = WebGPU._tableGet(bufferPtr).object;
    commandEncoder.clearBuffer(buffer, offset, size);
  },

  wgpuCommandEncoderCopyBufferToBuffer: (encoderPtr, srcPtr, srcOffset, dstPtr, dstOffset, size) => {
    var commandEncoder = WebGPU._tableGet(encoderPtr);
    var src = WebGPU._tableGet(srcPtr).object;
    var dst = WebGPU._tableGet(dstPtr).object;
    commandEncoder.copyBufferToBuffer(src, srcOffset, dst, dstOffset, size);
  },

  wgpuCommandEncoderCopyBufferToTexture: (encoderPtr, srcPtr, dstPtr, copySizePtr) => {
    var commandEncoder = WebGPU._tableGet(encoderPtr);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder.copyBufferToTexture(
      WebGPU.makeImageCopyBuffer(srcPtr), WebGPU.makeImageCopyTexture(dstPtr), copySize);
  },

  wgpuCommandEncoderCopyTextureToBuffer: (encoderPtr, srcPtr, dstPtr, copySizePtr) => {
    var commandEncoder = WebGPU._tableGet(encoderPtr);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder.copyTextureToBuffer(
      WebGPU.makeImageCopyTexture(srcPtr), WebGPU.makeImageCopyBuffer(dstPtr), copySize);
  },

  wgpuCommandEncoderCopyTextureToTexture: (encoderPtr, srcPtr, dstPtr, copySizePtr) => {
    var commandEncoder = WebGPU._tableGet(encoderPtr);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder.copyTextureToTexture(
      WebGPU.makeImageCopyTexture(srcPtr), WebGPU.makeImageCopyTexture(dstPtr), copySize);
  },

  wgpuCommandEncoderFinish__deps: ['emwgpuCreateCommandBuffer'],
  wgpuCommandEncoderFinish: (encoderPtr, descriptor) => {
    // TODO: Use the descriptor.
    var commandEncoder = WebGPU._tableGet(encoderPtr);
    var ptr = _emwgpuCreateCommandBuffer();
    WebGPU._tableInsert(ptr, commandEncoder.finish());
    return ptr;
  },

  wgpuCommandEncoderInsertDebugMarker: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.insertDebugMarker(UTF8ToString(markerLabelPtr));
  },

  wgpuCommandEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuCommandEncoderPushDebugGroup: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.pushDebugGroup(UTF8ToString(groupLabelPtr));
  },

  wgpuCommandEncoderResolveQuerySet: (encoderPtr, querySetPtr, firstQuery, queryCount, destinationPtr, destinationOffset) => {
    var commandEncoder = WebGPU._tableGet(encoderPtr);
    var querySet = WebGPU._tableGet(querySetPtr);
    var destination = WebGPU._tableGet(destinationPtr).object;

    commandEncoder.resolveQuerySet(querySet, firstQuery, queryCount, destination, destinationOffset);
  },

  wgpuCommandEncoderSetLabel: (encoderPtr, labelPtr) => {
    var commandEncoder = WebGPU._tableGet(encoderPtr);
    commandEncoder.label = UTF8ToString(labelPtr);
  },

  wgpuCommandEncoderWriteTimestamp: (encoderPtr, querySetPtr, queryIndex) => {
    var commandEncoder = WebGPU._tableGet(encoderPtr);
    var querySet = WebGPU._tableGet(querySetPtr);
    commandEncoder.writeTimestamp(querySet, queryIndex);
  },

  // --------------------------------------------------------------------------
  // Methods of ComputePassEncoder
  // --------------------------------------------------------------------------

  wgpuComputePassEncoderDispatchWorkgroups: (passPtr, x, y, z) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.dispatchWorkgroups(x, y, z);
  },

  wgpuComputePassEncoderDispatchWorkgroupsIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var indirectBuffer = WebGPU._tableGet(indirectBufferPtr).object;
    var pass = WebGPU._tableGet(passPtr);
    pass.dispatchWorkgroupsIndirect(indirectBuffer, indirectOffset);
  },

  wgpuComputePassEncoderEnd: (passPtr) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.end();
  },

  wgpuComputePassEncoderInsertDebugMarker: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.insertDebugMarker(UTF8ToString(markerLabelPtr));
  },

  wgpuComputePassEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuComputePassEncoderPushDebugGroup: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.pushDebugGroup(UTF8ToString(groupLabelPtr));
  },

  wgpuComputePassEncoderSetBindGroup: (passPtr, groupIndex, groupPtr, dynamicOffsetCount, dynamicOffsetsPtr) => {
    var pass = WebGPU._tableGet(passPtr);
    var group = WebGPU._tableGet(groupPtr);
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

  wgpuComputePassEncoderSetLabel: (passPtr, labelPtr) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.label = UTF8ToString(labelPtr);
  },

  wgpuComputePassEncoderSetPipeline: (passPtr, pipelinePtr) => {
    var pass = WebGPU._tableGet(passPtr);
    var pipeline = WebGPU._tableGet(pipelinePtr);
    pass.setPipeline(pipeline);
  },

  wgpuComputePassEncoderWriteTimestamp: (encoderPtr, querySetPtr, queryIndex) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    var querySet = WebGPU._tableGet(querySetPtr);
    encoder.writeTimestamp(querySet, queryIndex);
  },

  // --------------------------------------------------------------------------
  // Methods of ComputePipeline
  // --------------------------------------------------------------------------

  wgpuComputePipelineGetBindGroupLayout__deps: ['emwgpuCreateBindGroupLayout'],
  wgpuComputePipelineGetBindGroupLayout: (pipelinePtr, groupIndex) => {
    var pipeline = WebGPU._tableGet(pipelinePtr);
    var ptr = _emwgpuCreateBindGroupLayout();
    WebGPU._tableInsert(ptr, pipeline.getBindGroupLayout(groupIndex));
    return ptr;
  },

  wgpuComputePipelineSetLabel: (pipelinePtr, labelPtr) => {
    var pipeline = WebGPU._tableGet(pipelinePtr);
    pipeline.label = UTF8ToString(labelPtr);
  },

  // --------------------------------------------------------------------------
  // Methods of Device
  // --------------------------------------------------------------------------

  wgpuDeviceCreateBindGroup__deps: ['$readI53FromI64', 'emwgpuCreateBindGroup'],
  wgpuDeviceCreateBindGroup: (devicePtr, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makeEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var bufferPtr = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.buffer) }}};
      var samplerPtr = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.sampler) }}};
      var textureViewPtr = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.textureView) }}};
#if ASSERTIONS
      assert((bufferPtr !== 0) + (samplerPtr !== 0) + (textureViewPtr !== 0) === 1);
#endif

      var binding = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.binding) }}};

      if (bufferPtr) {
        var size = {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupEntry.size, 'i53') }}};
        {{{ gpu.convertSentinelToUndefined('size') }}}

        return {
          "binding": binding,
          "resource": {
            "buffer": WebGPU._tableGet(bufferPtr).object,
            "offset": {{{ gpu.makeGetU64('entryPtr', C_STRUCTS.WGPUBindGroupEntry.offset) }}},
            "size": size
          },
        };
      } else if (samplerPtr) {
        return {
          "binding": binding,
          "resource": WebGPU._tableGet(samplerPtr),
        };
      } else {
        return {
          "binding": binding,
          "resource": WebGPU._tableGet(textureViewPtr),
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
      "layout": WebGPU._tableGet(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.layout, '*') }}}),
      "entries": makeEntries(
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.entryCount) }}},
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.entries, '*') }}}
      ),
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU._tableGet(devicePtr);
    var ptr = _emwgpuCreateBindGroup();
    WebGPU._tableInsert(ptr, device.createBindGroup(desc));
    return ptr;
  },

  wgpuDeviceCreateBindGroupLayout__deps: ['emwgpuCreateBindGroupLayout'],
  wgpuDeviceCreateBindGroupLayout: (devicePtr, descriptor) => {
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

    var device = WebGPU._tableGet(devicePtr);
    var ptr = _emwgpuCreateBindGroupLayout();
    WebGPU._tableInsert(ptr, device.createBindGroupLayout(desc));
    return ptr;
  },

  wgpuDeviceCreateBuffer__deps: ['emwgpuCreateBuffer'],
  wgpuDeviceCreateBuffer: (devicePtr, descriptor) => {
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

    var device = WebGPU._tableGet(devicePtr);
    var bufferWrapper = {
      object: device.createBuffer(desc),
    };
    var ptr = _emwgpuCreateBuffer();
    WebGPU._tableInsert(ptr, bufferWrapper);
    if (mappedAtCreation) {
      bufferWrapper.mapMode = {{{ gpu.MapMode.Write }}};
      bufferWrapper.onUnmap = [];
    }
    return ptr;
  },

  wgpuDeviceCreateCommandEncoder__deps: ['emwgpuCreateCommandEncoder'],
  wgpuDeviceCreateCommandEncoder: (devicePtr, descriptor) => {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {
        "label": undefined,
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUCommandEncoderDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }
    var device = WebGPU._tableGet(devicePtr);
    var ptr = _emwgpuCreateCommandEncoder();
    WebGPU._tableInsert(ptr, device.createCommandEncoder(desc));
    return ptr;
  },

  wgpuDeviceCreateComputePipeline__deps: ['emwgpuCreateComputePipeline'],
  wgpuDeviceCreateComputePipeline: (devicePtr, descriptor) => {
    var desc = WebGPU.makeComputePipelineDesc(descriptor);
    var device = WebGPU._tableGet(devicePtr);
    var ptr = _emwgpuCreateComputePipeline();
    WebGPU._tableInsert(ptr, device.createComputePipeline(desc));
    return ptr;
  },

  emwgpuDeviceCreateComputePipelineAsync__i53abi:false,
  emwgpuDeviceCreateComputePipelineAsync__deps: ['$stringToUTF8OnStack', 'emwgpuCreateComputePipeline', 'emwgpuOnDeviceCreateComputePipelineCompleted'],
  emwgpuDeviceCreateComputePipelineAsync: (devicePtr, futureIdL, futureIdH, descriptor) => {
    var desc = WebGPU.makeComputePipelineDesc(descriptor);
    var device = WebGPU._tableGet(devicePtr);
    {{{ runtimeKeepalivePush() }}}
    WebGPU._futureInsert(futureIdL, futureIdH, device.createComputePipelineAsync(desc).then((pipeline) => {
      {{{ runtimeKeepalivePop() }}}
      var pipelinePtr = _emwgpuCreateComputePipeline();
      WebGPU._tableInsert(pipelinePtr, pipeline);
      _emwgpuOnDeviceCreateComputePipelineCompleted(futureIdL, futureIdH, {{{ gpu.CreatePipelineAsyncStatus.Success }}}, pipelinePtr, 0);
    }, (pipelineError) => {
      {{{ runtimeKeepalivePop() }}}
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack(pipelineError.message);
      var status =
        pipeline.reason === 'validation' ? {{{ gpu.CreatePipelineAsyncStatus.ValidationError }}} :
        pipeline.reason === 'internal' ? {{{ gpu.CreatePipelineAsyncStatus.InternalError }}} :
        {{{ gpu.CreatePipelineAsyncStatus.Unknown }}};
      _emwgpuOnDeviceCreateComputePipelineCompleted(futureIdL, futureIdH, status, 0, messagePtr);
      stackRestore(sp);
    }));
  },

  wgpuDeviceCreatePipelineLayout__deps: ['emwgpuCreatePipelineLayout'],
  wgpuDeviceCreatePipelineLayout: (devicePtr, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}
    var bglCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.bindGroupLayoutCount) }}};
    var bglPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.bindGroupLayouts, '*') }}};
    var bgls = [];
    for (var i = 0; i < bglCount; ++i) {
      bgls.push(WebGPU._tableGet(
        {{{ makeGetValue('bglPtr', `${POINTER_SIZE} * i`, '*') }}}));
    }
    var desc = {
      "label": undefined,
      "bindGroupLayouts": bgls,
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU._tableGet(devicePtr);
    var ptr = _emwgpuCreatePipelineLayout();
    WebGPU._tableInsert(ptr, device.createPipelineLayout(desc));
    return ptr;
  },

  wgpuDeviceCreateQuerySet__deps: ['emwgpuCreateQuerySet'],
  wgpuDeviceCreateQuerySet: (devicePtr, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var desc = {
      "type": WebGPU.QueryType[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.type) }}}],
      "count": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.count) }}},
    };

    var device = WebGPU._tableGet(devicePtr);
    var ptr = _emwgpuCreateQuerySet();
    WebGPU._tableInsert(ptr, device.createQuerySet(desc));
    return ptr;
  },

  wgpuDeviceCreateRenderBundleEncoder__deps: ['emwgpuCreateRenderBundleEncoder'],
  wgpuDeviceCreateRenderBundleEncoder: (devicePtr, descriptor) => {
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
    var device = WebGPU._tableGet(devicePtr);
    var ptr = _emwgpuCreateRenderBundleEncoder();
    WebGPU._tableInsert(ptr, device.createRenderBundleEncoder(desc));
    return ptr;
  },

  wgpuDeviceCreateRenderPipeline__deps: ['emwgpuCreateRenderPipeline'],
  wgpuDeviceCreateRenderPipeline: (devicePtr, descriptor) => {
    var desc = WebGPU.makeRenderPipelineDesc(descriptor);
    var device = WebGPU._tableGet(devicePtr);
    var ptr = _emwgpuCreateRenderPipeline();
    WebGPU._tableInsert(ptr, device.createRenderPipeline(desc));
    return ptr;
  },

  emwgpuDeviceCreateRenderPipelineAsync__i53abi:false,
  emwgpuDeviceCreateRenderPipelineAsync__deps: ['$stringToUTF8OnStack', 'emwgpuCreateRenderPipeline', 'emwgpuOnDeviceCreateRenderPipelineCompleted'],
  emwgpuDeviceCreateRenderPipelineAsync: (devicePtr, futureIdL, futureIdH, descriptor) => {
    var desc = WebGPU.makeRenderPipelineDesc(descriptor);
    var device = WebGPU._tableGet(devicePtr);
    {{{ runtimeKeepalivePush() }}}
    WebGPU._futureInsert(futureIdL, futureIdH, device.createRenderPipelineAsync(desc).then((pipeline) => {
      {{{ runtimeKeepalivePop() }}}
      var pipelinePtr = _emwgpuCreateRenderPipeline();
      WebGPU._tableInsert(pipelinePtr, pipeline);
      _emwgpuOnDeviceCreateRenderPipelineCompleted(futureIdL, futureIdH, {{{ gpu.CreatePipelineAsyncStatus.Success }}}, pipelinePtr, 0);
    }, (pipelineError) => {
      {{{ runtimeKeepalivePop() }}}
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack(pipelineError.message);
      var status =
        pipeline.reason === 'validation' ? {{{ gpu.CreatePipelineAsyncStatus.ValidationError }}} :
        pipeline.reason === 'internal' ? {{{ gpu.CreatePipelineAsyncStatus.InternalError }}} :
        {{{ gpu.CreatePipelineAsyncStatus.Unknown }}};
        _emwgpuOnDeviceCreateRenderPipelineCompleted(futureIdL, futureIdH, status, 0, messagePtr);
      stackRestore(sp);
    }));
  },

  wgpuDeviceCreateSampler__deps: ['emwgpuCreateSampler'],
  wgpuDeviceCreateSampler: (devicePtr, descriptor) => {
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

    var device = WebGPU._tableGet(devicePtr);
    var ptr = _emwgpuCreateSampler();
    WebGPU._tableInsert(ptr, device.createSampler(desc));
    return ptr;
  },

  wgpuDeviceCreateShaderModule__deps: ['emwgpuCreateShaderModule'],
  wgpuDeviceCreateShaderModule: (devicePtr, descriptor) => {
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
      case {{{ gpu.SType.ShaderSourceSPIRV }}}: {
        var count = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUShaderSourceSPIRV.codeSize) }}};
        var start = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUShaderSourceSPIRV.code, '*') }}};
        var offset = {{{ getHeapOffset('start', 'u32') }}};
#if PTHREADS
        // Chrome can't currently handle a SharedArrayBuffer view here, so make a copy.
        desc["code"] = HEAPU32.slice(offset, offset + count);
#else
        desc["code"] = HEAPU32.subarray(offset, offset + count);
#endif
        break;
      }
      case {{{ gpu.SType.ShaderSourceWGSL }}}: {
        var sourcePtr = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUShaderSourceWGSL.code, '*') }}};
        if (sourcePtr) {
          desc["code"] = UTF8ToString(sourcePtr);
        }
        break;
      }
#if ASSERTIONS
      default: abort('unrecognized ShaderModule sType');
#endif
    }

    var device = WebGPU._tableGet(devicePtr);
    var ptr = _emwgpuCreateShaderModule();
    WebGPU._tableInsert(ptr, device.createShaderModule(desc));
    return ptr;
  },

  wgpuDeviceCreateTexture__deps: ['emwgpuCreateTexture'],
  wgpuDeviceCreateTexture: (devicePtr, descriptor) => {
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

    var device = WebGPU._tableGet(devicePtr);
    var ptr = _emwgpuCreateTexture();
    WebGPU._tableInsert(ptr, device.createTexture(desc));
    return ptr;
  },

  wgpuDeviceDestroy: (devicePtr) => {
    WebGPU._tableGet(devicePtr).destroy()
  },

  wgpuDeviceEnumerateFeatures: (devicePtr, featuresOutPtr) => {
    var device = WebGPU._tableGet(devicePtr);
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

  wgpuDeviceGetLimits: (devicePtr, limitsOutPtr) => {
    var device = WebGPU._tableGet(devicePtr);
    WebGPU.fillLimitStruct(device.limits, limitsOutPtr);
    return 1;
  },

  wgpuDeviceHasFeature: (devicePtr, featureEnumValue) => {
    var device = WebGPU._tableGet(devicePtr);
    return device.features.has(WebGPU.FeatureName[featureEnumValue]);
  },

  wgpuDevicePopErrorScope__deps: ['$callUserCallback'],
  wgpuDevicePopErrorScope: (devicePtr, callback, userdata) => {
    var device = WebGPU._tableGet(devicePtr);
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

  wgpuDevicePushErrorScope: (devicePtr, filter) => {
    var device = WebGPU._tableGet(devicePtr);
    device.pushErrorScope(WebGPU.ErrorFilter[filter]);
  },

  wgpuDeviceSetLabel: (devicePtr, labelPtr) => {
    var device = WebGPU._tableGet(devicePtr);
    device.label = UTF8ToString(labelPtr);
  },

  // TODO(42241415) Remove this after verifying that it's not used and/or updating users.
  wgpuDeviceSetUncapturedErrorCallback__deps: ['$callUserCallback'],
  wgpuDeviceSetUncapturedErrorCallback: (devicePtr, callback, userdata) => {
    var device = WebGPU._tableGet(devicePtr);
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

  // --------------------------------------------------------------------------
  // Methods of Instance
  // --------------------------------------------------------------------------

  wgpuInstanceCreateSurface__deps: ['$findCanvasEventTarget', 'emwgpuCreateSurface'],
  wgpuInstanceCreateSurface: (instancePtr, descriptor) => {
    {{{ gpu.makeCheck('descriptor') }}}
    var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSurfaceDescriptor.nextInChain, '*') }}};
#if ASSERTIONS
    assert(nextInChainPtr !== 0);
    assert({{{ gpu.SType.SurfaceSourceCanvasHTMLSelector_Emscripten }}} ===
      {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}});
#endif
    var sourceCanvasHTMLSelector = nextInChainPtr;

    {{{ gpu.makeCheckDescriptor('sourceCanvasHTMLSelector') }}}
    var selectorPtr = {{{ makeGetValue('sourceCanvasHTMLSelector', C_STRUCTS.WGPUSurfaceSourceCanvasHTMLSelector_Emscripten.selector, '*') }}};
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

    var ptr = _emwgpuCreateSurface();
    WebGPU._tableInsert(ptr, context);
    return ptr;
  },

  wgpuInstanceHasWGSLLanguageFeature: (instance, featureEnumValue) => {
    if (!('wgslLanguageFeatures' in navigator["gpu"])) {
      return false;
    }
    return navigator["gpu"]["wgslLanguageFeatures"].has(WebGPU.WGSLFeatureName[featureEnumValue]);
  },

  emwgpuInstanceRequestAdapter__i53abi: false,
  emwgpuInstanceRequestAdapter__deps: ['$callUserCallback', '$stringToUTF8OnStack', 'emwgpuCreateAdapter', 'emwgpuOnRequestAdapterCompleted'],
  emwgpuInstanceRequestAdapter: (instancePtr, futureIdL, futureIdH, options) => {
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
      _emwgpuOnRequestAdapterCompleted(futureIdL, futureIdH, {{{ gpu.RequestAdapterStatus.Unavailable }}}, 0, messagePtr);
      stackRestore(sp);
      return;
    }

    {{{ runtimeKeepalivePush() }}}
    WebGPU._futureInsert(futureIdL, futureIdH, navigator["gpu"]["requestAdapter"](opts).then((adapter) => {
      {{{ runtimeKeepalivePop() }}}
      if (adapter) {
        var adapterPtr = _emwgpuCreateAdapter(instancePtr);
        WebGPU._tableInsert(adapterPtr, adapter);
        _emwgpuOnRequestAdapterCompleted(futureIdL, futureIdH, {{{ gpu.RequestAdapterStatus.Success }}}, adapterPtr, 0);
      } else {
        var sp = stackSave();
        var messagePtr = stringToUTF8OnStack('WebGPU not available on this browser (requestAdapter returned null)');
        _emwgpuOnRequestAdapterCompleted(futureIdL, futureIdH, {{{ gpu.RequestAdapterStatus.Unavailable }}}, 0, messagePtr);
        stackRestore(sp);
      }
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}}
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack(ex.message);
      _emwgpuOnRequestAdapterCompleted(futureIdL, futureIdH, {{{ gpu.RequestAdapterStatus.Error }}}, 0, messagePtr);
      stackRestore(sp);
    }));
  },

  // --------------------------------------------------------------------------
  // Methods of PipelineLayout
  // --------------------------------------------------------------------------

  wgpuPipelineLayoutSetLabel: (pipelineLayoutPtr, labelPtr) => {
    var pipelineLayout = WebGPU._tableGet(pipelineLayoutPtr);
    pipelineLayout.label = UTF8ToString(labelPtr);
  },

  // --------------------------------------------------------------------------
  // Methods of QuerySet
  // --------------------------------------------------------------------------

  wgpuQuerySetDestroy: (querySetPtr) => {
    WebGPU._tableGet(querySetPtr).destroy();
  },

  wgpuQuerySetGetCount: (querySetPtr) => {
    var querySet = WebGPU._tableGet(querySetPtr);
    return querySet.count;
  },

  wgpuQuerySetGetType: (querySetPtr, labelPtr) => {
    var querySet = WebGPU._tableGet(querySetPtr);
    return querySet.type;
  },

  wgpuQuerySetSetLabel: (querySetPtr, labelPtr) => {
    var querySet = WebGPU._tableGet(querySetPtr);
    querySet.label = UTF8ToString(labelPtr);
  },

  // --------------------------------------------------------------------------
  // Methods of Queue
  // --------------------------------------------------------------------------

  wgpuQueueOnSubmittedWorkDone__deps: ['$callUserCallback'],
  wgpuQueueOnSubmittedWorkDone: (queuePtr, callback, userdata) => {
    var queue = WebGPU._tableGet(queuePtr);

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

  wgpuQueueSetLabel: (queuePtr, labelPtr) => {
    var queue = WebGPU._tableGet(queuePtr);
    queue.label = UTF8ToString(labelPtr);
  },

  wgpuQueueSubmit: (queuePtr, commandCount, commands) => {
#if ASSERTIONS
    assert(commands % 4 === 0);
#endif
    var queue = WebGPU._tableGet(queuePtr);
    var cmds = Array.from({{{ makeHEAPView(`${POINTER_BITS}`, 'commands', `commands + commandCount * ${POINTER_SIZE}`)}}},
      (id) => WebGPU._tableGet(id));
    queue.submit(cmds);
  },

  wgpuQueueWriteBuffer: (queuePtr, bufferPtr, bufferOffset, data, size) => {
    var queue = WebGPU._tableGet(queuePtr);
    var buffer = WebGPU._tableGet(bufferPtr).object;
    // There is a size limitation for ArrayBufferView. Work around by passing in a subarray
    // instead of the whole heap. crbug.com/1201109
    var subarray = HEAPU8.subarray(data, data + size);
    queue.writeBuffer(buffer, bufferOffset, subarray, 0, size);
  },

  wgpuQueueWriteTexture: (queuePtr, destinationPtr, data, dataSize, dataLayoutPtr, writeSizePtr) => {
    var queue = WebGPU._tableGet(queuePtr);

    var destination = WebGPU.makeImageCopyTexture(destinationPtr);
    var dataLayout = WebGPU.makeTextureDataLayout(dataLayoutPtr);
    var writeSize = WebGPU.makeExtent3D(writeSizePtr);
    // This subarray isn't strictly necessary, but helps work around an issue
    // where Chromium makes a copy of the entire heap. crbug.com/1134457
    var subarray = HEAPU8.subarray(data, data + dataSize);
    queue.writeTexture(destination, subarray, dataLayout, writeSize);
  },

  // --------------------------------------------------------------------------
  // Methods of RenderBundle
  // --------------------------------------------------------------------------

  wgpuRenderBundleSetLabel: (bundlePtr, labelPtr) => {
    var bundle = WebGPU._tableGet(bundlePtr);
    bundle.label = UTF8ToString(labelPtr);
  },

  // --------------------------------------------------------------------------
  // Methods of RenderBundleEncoder
  // --------------------------------------------------------------------------

  wgpuRenderBundleEncoderDraw: (passPtr, vertexCount, instanceCount, firstVertex, firstInstance) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.draw(vertexCount, instanceCount, firstVertex, firstInstance);
  },

  wgpuRenderBundleEncoderDrawIndexed: (passPtr, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.drawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
  },

  wgpuRenderBundleEncoderDrawIndexedIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var indirectBuffer = WebGPU._tableGet(indirectBufferPtr).object;
    var pass = WebGPU._tableGet(passPtr);
    pass.drawIndexedIndirect(indirectBuffer, indirectOffset);
  },

  wgpuRenderBundleEncoderDrawIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var indirectBuffer = WebGPU._tableGet(indirectBufferPtr).object;
    var pass = WebGPU._tableGet(passPtr);
    pass.drawIndirect(indirectBuffer, indirectOffset);
  },

  wgpuRenderBundleEncoderFinish__deps: ['emwgpuCreateRenderBundle'],
  wgpuRenderBundleEncoderFinish: (encoderPtr, descriptor) => {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {};
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderBundleDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }
    var encoder = WebGPU._tableGet(encoderPtr);
    var ptr = _emwgpuCreateRenderBundle();
    WebGPU._tableInsert(ptr, encoder.finish(desc));
    return ptr;
  },

  wgpuRenderBundleEncoderInsertDebugMarker: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.insertDebugMarker(UTF8ToString(markerLabelPtr));
  },

  wgpuRenderBundleEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuRenderBundleEncoderPushDebugGroup: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.pushDebugGroup(UTF8ToString(groupLabelPtr));
  },

  wgpuRenderBundleEncoderSetBindGroup: (passPtr, groupIndex, groupPtr, dynamicOffsetCount, dynamicOffsetsPtr) => {
    var pass = WebGPU._tableGet(passPtr);
    var group = WebGPU._tableGet(groupPtr);
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

  wgpuRenderBundleEncoderSetIndexBuffer: (passPtr, bufferPtr, format, offset, size) => {
    var pass = WebGPU._tableGet(passPtr);
    var buffer = WebGPU._tableGet(bufferPtr).object;
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setIndexBuffer(buffer, WebGPU.IndexFormat[format], offset, size);
  },

  wgpuRenderBundleEncoderSetLabel: (passPtr, labelPtr) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.label = UTF8ToString(labelPtr);
  },

  wgpuRenderBundleEncoderSetPipeline: (passPtr, pipelinePtr) => {
    var pass = WebGPU._tableGet(passPtr);
    var pipeline = WebGPU._tableGet(pipelinePtr);
    pass.setPipeline(pipeline);
  },

  wgpuRenderBundleEncoderSetVertexBuffer: (passPtr, slot, bufferPtr, offset, size) => {
    var pass = WebGPU._tableGet(passPtr);
    var buffer = WebGPU._tableGet(bufferPtr).object;
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setVertexBuffer(slot, buffer, offset, size);
  },

  // --------------------------------------------------------------------------
  // Methods of RenderPassEncoder
  // --------------------------------------------------------------------------

  wgpuRenderPassEncoderBeginOcclusionQuery: (passPtr, queryIndex) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.beginOcclusionQuery(queryIndex);
  },

  wgpuRenderPassEncoderDraw: (passPtr, vertexCount, instanceCount, firstVertex, firstInstance) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.draw(vertexCount, instanceCount, firstVertex, firstInstance);
  },

  wgpuRenderPassEncoderDrawIndexed: (passPtr, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.drawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
  },

  wgpuRenderPassEncoderDrawIndexedIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var indirectBuffer = WebGPU._tableGet(indirectBufferPtr).object;
    var pass = WebGPU._tableGet(passPtr);
    pass.drawIndexedIndirect(indirectBuffer, indirectOffset);
  },

  wgpuRenderPassEncoderDrawIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var indirectBuffer = WebGPU._tableGet(indirectBufferPtr).object;
    var pass = WebGPU._tableGet(passPtr);
    pass.drawIndirect(indirectBuffer, indirectOffset);
  },

  wgpuRenderPassEncoderEnd: (encoderPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.end();
  },

  wgpuRenderPassEncoderEndOcclusionQuery: (passPtr) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.endOcclusionQuery();
  },

  wgpuRenderPassEncoderExecuteBundles: (passPtr, count, bundlesPtr) => {
    var pass = WebGPU._tableGet(passPtr);

#if ASSERTIONS
    assert(bundlesPtr % 4 === 0);
#endif

    var bundles = Array.from({{{ makeHEAPView(`${POINTER_BITS}`, 'bundlesPtr', `bundlesPtr + count * ${POINTER_SIZE}`) }}},
      (id) => WebGPU._tableGet(id));
    pass.executeBundles(bundles);
  },

  wgpuRenderPassEncoderInsertDebugMarker: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.insertDebugMarker(UTF8ToString(markerLabelPtr));
  },

  wgpuRenderPassEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuRenderPassEncoderPushDebugGroup: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    encoder.pushDebugGroup(UTF8ToString(groupLabelPtr));
  },

  wgpuRenderPassEncoderSetBindGroup: (passPtr, groupIndex, groupPtr, dynamicOffsetCount, dynamicOffsetsPtr) => {
    var pass = WebGPU._tableGet(passPtr);
    var group = WebGPU._tableGet(groupPtr);
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

  wgpuRenderPassEncoderSetBlendConstant: (passPtr, colorPtr) => {
    var pass = WebGPU._tableGet(passPtr);
    var color = WebGPU.makeColor(colorPtr);
    pass.setBlendConstant(color);
  },

  wgpuRenderPassEncoderSetIndexBuffer: (passPtr, bufferPtr, format, offset, size) => {
    var pass = WebGPU._tableGet(passPtr);
    var buffer = WebGPU._tableGet(bufferPtr).object;
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setIndexBuffer(buffer, WebGPU.IndexFormat[format], offset, size);
  },

  wgpuRenderPassEncoderSetLabel: (passPtr, labelPtr) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.label = UTF8ToString(labelPtr);
  },

  wgpuRenderPassEncoderSetPipeline: (passPtr, pipelinePtr) => {
    var pass = WebGPU._tableGet(passPtr);
    var pipeline = WebGPU._tableGet(pipelinePtr);
    pass.setPipeline(pipeline);
  },

  wgpuRenderPassEncoderSetScissorRect: (passPtr, x, y, w, h) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.setScissorRect(x, y, w, h);
  },

  wgpuRenderPassEncoderSetStencilReference: (passPtr, reference) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.setStencilReference(reference);
  },

  wgpuRenderPassEncoderSetVertexBuffer: (passPtr, slot, bufferPtr, offset, size) => {
    var pass = WebGPU._tableGet(passPtr);
    var buffer = WebGPU._tableGet(bufferPtr).object;
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setVertexBuffer(slot, buffer, offset, size);
  },

  wgpuRenderPassEncoderSetViewport: (passPtr, x, y, w, h, minDepth, maxDepth) => {
    var pass = WebGPU._tableGet(passPtr);
    pass.setViewport(x, y, w, h, minDepth, maxDepth);
  },

  wgpuRenderPassEncoderWriteTimestamp: (encoderPtr, querySetPtr, queryIndex) => {
    var encoder = WebGPU._tableGet(encoderPtr);
    var querySet = WebGPU._tableGet(querySetPtr);
    encoder.writeTimestamp(querySet, queryIndex);
  },

  // --------------------------------------------------------------------------
  // Methods of RenderPipeline
  // --------------------------------------------------------------------------

  wgpuRenderPipelineGetBindGroupLayout__deps: ['emwgpuCreateBindGroupLayout'],
  wgpuRenderPipelineGetBindGroupLayout: (pipelinePtr, groupIndex) => {
    var pipeline = WebGPU._tableGet(pipelinePtr);
    var ptr = _emwgpuCreateBindGroupLayout();
    WebGPU._tableInsert(ptr, pipeline.getBindGroupLayout(groupIndex));
  },

  wgpuRenderPipelineSetLabel: (pipelinePtr, labelPtr) => {
    var pipeline = WebGPU._tableGet(pipelinePtr);
    pipeline.label = UTF8ToString(labelPtr);
  },

  // --------------------------------------------------------------------------
  // Methods of Sampler
  // --------------------------------------------------------------------------

  wgpuSamplerSetLabel: (samplerPtr, labelPtr) => {
    var sampler = WebGPU._tableGet(samplerPtr);
    sampler.label = UTF8ToString(labelPtr);
  },

  // --------------------------------------------------------------------------
  // Methods of ShaderModule
  // --------------------------------------------------------------------------

  wgpuShaderModuleGetCompilationInfo__deps: ['$callUserCallback', '$stringToUTF8', '$lengthBytesUTF8', 'malloc', 'free'],
  wgpuShaderModuleGetCompilationInfo: (shaderModulePtr, callback, userdata) => {
    var shaderModule = WebGPU._tableGet(shaderModulePtr);
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

  wgpuShaderModuleSetLabel: (shaderModulePtr, labelPtr) => {
    var shaderModule = WebGPU._tableGet(shaderModulePtr);
    shaderModule.label = UTF8ToString(labelPtr);
  },

  // --------------------------------------------------------------------------
  // Methods of Surface
  // --------------------------------------------------------------------------

  wgpuSurfaceConfigure: (surfacePtr, config) => {
    {{{ gpu.makeCheckDescriptor('config') }}}
    var devicePtr = {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.device, '*') }}};
    var context = WebGPU._tableGet(surfacePtr);

#if ASSERTIONS
    var viewFormatCount = {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.viewFormatCount) }}};
    var viewFormats = {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.viewFormats, '*') }}};
    assert(viewFormatCount === 0 && viewFormats === 0, "TODO: Support viewFormats.");
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
      "device": WebGPU._tableGet(devicePtr),
      "format": WebGPU.TextureFormat[
        {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.format) }}}],
      "usage": {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.usage) }}},
      "alphaMode": WebGPU.CompositeAlphaMode[
        {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.alphaMode) }}}],
    };
    context.configure(configuration);
  },

  wgpuSurfaceGetCurrentTexture__deps: ['emwgpuCreateTexture'],
  wgpuSurfaceGetCurrentTexture: (surfacePtr, surfaceTexturePtr) => {
    {{{ gpu.makeCheck('surfaceTexturePtr') }}}
    var context = WebGPU._tableGet(surfacePtr);

    try {
      var texturePtr = _emwgpuCreateTexture();
      WebGPU._tableInsert(texturePtr, context.getCurrentTexture());
      {{{ makeSetValue('surfaceTexturePtr', C_STRUCTS.WGPUSurfaceTexture.texture, 'texturePtr', '*') }}};
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

  wgpuSurfacePresent: (surfacePtr) => {
    // TODO: This could probably be emulated with ASYNCIFY.
    abort('wgpuSurfacePresent is unsupported (use requestAnimationFrame via html5.h instead)');
  },

  wgpuSurfaceUnconfigure: (surfacePtr) => {
    var context = WebGPU._tableGet(surfacePtr);
    context.unconfigure();
  },

  // --------------------------------------------------------------------------
  // Methods of Texture
  // --------------------------------------------------------------------------

  wgpuTextureCreateView__deps: ['emwgpuCreateTextureView'],
  wgpuTextureCreateView: (texturePtr, descriptor) => {
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

    var texture = WebGPU._tableGet(texturePtr);
    var ptr = _emwgpuCreateTextureView();
    WebGPU._tableInsert(ptr, texture.createView(desc));
    return ptr;
  },

  wgpuTextureDestroy: (texturePtr) => {
    WebGPU._tableGet(texturePtr).destroy();
  },

  wgpuTextureGetDepthOrArrayLayers: (texturePtr) => {
    var texture = WebGPU._tableGet(texturePtr);
    return texture.depthOrArrayLayers;
  },

  wgpuTextureGetDimension: (texturePtr) => {
    var texture = WebGPU._tableGet(texturePtr);
    return WebGPU.TextureDimension.indexOf(texture.dimension);
  },

  wgpuTextureGetFormat: (texturePtr) => {
    var texture = WebGPU._tableGet(texturePtr);
    // Should return the enum integer instead of string.
    return WebGPU.TextureFormat.indexOf(texture.format);
  },

  wgpuTextureGetHeight: (texturePtr) => {
    var texture = WebGPU._tableGet(texturePtr);
    return texture.height;
  },

  wgpuTextureGetMipLevelCount: (texturePtr) => {
    var texture = WebGPU._tableGet(texturePtr);
    return texture.mipLevelCount;
  },

  wgpuTextureGetSampleCount: (texturePtr) => {
    var texture = WebGPU._tableGet(texturePtr);
    return texture.sampleCount;
  },

  wgpuTextureGetUsage: (texturePtr) => {
    var texture = WebGPU._tableGet(texturePtr);
    return texture.usage;
  },

  wgpuTextureGetWidth: (texturePtr) => {
    var texture = WebGPU._tableGet(texturePtr);
    return texture.width;
  },

  wgpuTextureSetLabel: (texturePtr, labelPtr) => {
    var texture = WebGPU._tableGet(texturePtr);
    texture.label = UTF8ToString(labelPtr);
  },

  // --------------------------------------------------------------------------
  // Methods of TextureView
  // --------------------------------------------------------------------------

  wgpuTextureViewSetLabel: (textureViewPtr, labelPtr) => {
    var textureView = WebGPU._tableGet(textureViewPtr);
    textureView.label = UTF8ToString(labelPtr);
  },
};

// Inverted index used by EnumerateFeatures/HasFeature
LibraryWebGPU.$WebGPU.FeatureNameString2Enum = {};
for (var value in LibraryWebGPU.$WebGPU.FeatureName) {
  LibraryWebGPU.$WebGPU.FeatureNameString2Enum[LibraryWebGPU.$WebGPU.FeatureName[value]] = value;
}

for (const key of Object.keys(LibraryWebGPU)) {
  if (typeof LibraryWebGPU[key] === 'function') {
    if (!(key + '__i53abi' in LibraryWebGPU)) {
      LibraryWebGPU[key + '__i53abi'] = true;
    }
  }
}

autoAddDeps(LibraryWebGPU, '$WebGPU');
mergeInto(LibraryManager.library, LibraryWebGPU);
