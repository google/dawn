//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct tint_TransferFunctionParams {
  uint mode;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  float G;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_TransferFunctionParams srcTransferFunction;
  tint_TransferFunctionParams dstTransferFunction;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 apparentSize;
  vec2 plane1CoordFactor;
};

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v_1;
layout(binding = 4, std140)
uniform f_arg_0_params_block_ubo {
  uvec4 inner[17];
} v_2;
uniform highp sampler2D f_arg_0_plane0_arg_1;
uniform highp sampler2D f_arg_0_plane1_arg_1;
vec3 tint_ApplyGammaTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  vec3 v_3 = vec3(params.G);
  vec3 v_4 = abs(v);
  vec3 v_5 = sign(v);
  return mix((v_5 * (pow(((params.A * v_4) + params.B), v_3) + params.E)), (v_5 * ((params.C * v_4) + params.F)), lessThan(v_4, vec3(params.D)));
}
float tint_ApplyHLGSingleChannel(float v, tint_TransferFunctionParams params) {
  if ((v <= params.D)) {
    return ((v * v) / params.E);
  } else {
    return ((params.B + exp(((v - params.C) / params.A))) / params.F);
  }
  /* unreachable */
  return 0.0f;
}
vec3 tint_ApplyHLGTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  float v_6 = tint_ApplyHLGSingleChannel(v.x, params);
  float v_7 = tint_ApplyHLGSingleChannel(v.y, params);
  return vec3(v_6, v_7, tint_ApplyHLGSingleChannel(v.z, params));
}
vec3 tint_ApplyPQTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  vec3 v_8 = vec3(params.C);
  vec3 v_9 = vec3(params.D);
  vec3 v_10 = vec3(params.E);
  vec3 v_11 = vec3(params.A);
  vec3 v_12 = pow(clamp(v, vec3(0.0f), vec3(1.0f)), (vec3(1.0f) / vec3(params.B)));
  return pow((max((v_12 - v_8), vec3(0.0f)) / (v_9 - (v_10 * v_12))), (vec3(1.0f) / v_11));
}
vec3 tint_ApplySrcTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  if ((params.mode == 0u)) {
    return tint_ApplyGammaTransferFunction(v, params);
  } else {
    if ((params.mode == 1u)) {
      return tint_ApplyHLGTransferFunction(v, params);
    } else {
      return tint_ApplyPQTransferFunction(v, params);
    }
    /* unreachable */
    return vec3(0.0f);
  }
  /* unreachable */
  return vec3(0.0f);
}
vec4 tint_TextureSampleClampToEdgeMultiplanarExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_13 = (params.sampleTransform * vec3(coords, 1.0f));
  vec2 v_14 = clamp(v_13, params.samplePlane0RectMin, params.samplePlane0RectMax);
  vec3 v_15 = vec3(0.0f);
  float v_16 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_17 = textureLod(f_arg_0_plane0_arg_1, v_14, 0.0f);
    v_15 = v_17.xyz;
    v_16 = v_17.w;
  } else {
    v_15 = (vec4(textureLod(f_arg_0_plane0_arg_1, v_14, 0.0f).x, textureLod(f_arg_0_plane1_arg_1, clamp(v_13, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_16 = 1.0f;
  }
  vec3 v_18 = v_15;
  vec3 v_19 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_19 = tint_ApplyGammaTransferFunction((params.gamutConversionMatrix * tint_ApplySrcTransferFunction(v_18, params.srcTransferFunction)), params.dstTransferFunction);
  } else {
    v_19 = v_18;
  }
  return vec4(v_19, v_16);
}
mat3x2 v_20(uint start_byte_offset) {
  uvec4 v_21 = v_2.inner[(start_byte_offset / 16u)];
  vec2 v_22 = uintBitsToFloat(mix(v_21.xy, v_21.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_23 = (8u + start_byte_offset);
  uvec4 v_24 = v_2.inner[(v_23 / 16u)];
  vec2 v_25 = uintBitsToFloat(mix(v_24.xy, v_24.zw, bvec2((((v_23 & 15u) >> 2u) == 2u))));
  uint v_26 = (16u + start_byte_offset);
  uvec4 v_27 = v_2.inner[(v_26 / 16u)];
  return mat3x2(v_22, v_25, uintBitsToFloat(mix(v_27.xy, v_27.zw, bvec2((((v_26 & 15u) >> 2u) == 2u)))));
}
mat3 v_28(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_TransferFunctionParams v_29(uint start_byte_offset) {
  uvec4 v_30 = v_2.inner[(start_byte_offset / 16u)];
  uint v_31 = (4u + start_byte_offset);
  uvec4 v_32 = v_2.inner[(v_31 / 16u)];
  uint v_33 = (8u + start_byte_offset);
  uvec4 v_34 = v_2.inner[(v_33 / 16u)];
  uint v_35 = (12u + start_byte_offset);
  uvec4 v_36 = v_2.inner[(v_35 / 16u)];
  uint v_37 = (16u + start_byte_offset);
  uvec4 v_38 = v_2.inner[(v_37 / 16u)];
  uint v_39 = (20u + start_byte_offset);
  uvec4 v_40 = v_2.inner[(v_39 / 16u)];
  uint v_41 = (24u + start_byte_offset);
  uvec4 v_42 = v_2.inner[(v_41 / 16u)];
  uint v_43 = (28u + start_byte_offset);
  uvec4 v_44 = v_2.inner[(v_43 / 16u)];
  return tint_TransferFunctionParams(v_30[((start_byte_offset & 15u) >> 2u)], uintBitsToFloat(v_32[((v_31 & 15u) >> 2u)]), uintBitsToFloat(v_34[((v_33 & 15u) >> 2u)]), uintBitsToFloat(v_36[((v_35 & 15u) >> 2u)]), uintBitsToFloat(v_38[((v_37 & 15u) >> 2u)]), uintBitsToFloat(v_40[((v_39 & 15u) >> 2u)]), uintBitsToFloat(v_42[((v_41 & 15u) >> 2u)]), uintBitsToFloat(v_44[((v_43 & 15u) >> 2u)]));
}
mat3x4 v_45(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_46(uint start_byte_offset) {
  uvec4 v_47 = v_2.inner[(start_byte_offset / 16u)];
  uint v_48 = (4u + start_byte_offset);
  uvec4 v_49 = v_2.inner[(v_48 / 16u)];
  mat3x4 v_50 = v_45((16u + start_byte_offset));
  tint_TransferFunctionParams v_51 = v_29((64u + start_byte_offset));
  tint_TransferFunctionParams v_52 = v_29((96u + start_byte_offset));
  mat3 v_53 = v_28((128u + start_byte_offset));
  mat3x2 v_54 = v_20((176u + start_byte_offset));
  mat3x2 v_55 = v_20((200u + start_byte_offset));
  uint v_56 = (224u + start_byte_offset);
  uvec4 v_57 = v_2.inner[(v_56 / 16u)];
  vec2 v_58 = uintBitsToFloat(mix(v_57.xy, v_57.zw, bvec2((((v_56 & 15u) >> 2u) == 2u))));
  uint v_59 = (232u + start_byte_offset);
  uvec4 v_60 = v_2.inner[(v_59 / 16u)];
  vec2 v_61 = uintBitsToFloat(mix(v_60.xy, v_60.zw, bvec2((((v_59 & 15u) >> 2u) == 2u))));
  uint v_62 = (240u + start_byte_offset);
  uvec4 v_63 = v_2.inner[(v_62 / 16u)];
  vec2 v_64 = uintBitsToFloat(mix(v_63.xy, v_63.zw, bvec2((((v_62 & 15u) >> 2u) == 2u))));
  uint v_65 = (248u + start_byte_offset);
  uvec4 v_66 = v_2.inner[(v_65 / 16u)];
  vec2 v_67 = uintBitsToFloat(mix(v_66.xy, v_66.zw, bvec2((((v_65 & 15u) >> 2u) == 2u))));
  uint v_68 = (256u + start_byte_offset);
  uvec4 v_69 = v_2.inner[(v_68 / 16u)];
  uvec2 v_70 = mix(v_69.xy, v_69.zw, bvec2((((v_68 & 15u) >> 2u) == 2u)));
  uint v_71 = (264u + start_byte_offset);
  uvec4 v_72 = v_2.inner[(v_71 / 16u)];
  return tint_ExternalTextureParams(v_47[((start_byte_offset & 15u) >> 2u)], v_49[((v_48 & 15u) >> 2u)], v_50, v_51, v_52, v_53, v_54, v_55, v_58, v_61, v_64, v_67, v_70, uintBitsToFloat(mix(v_72.xy, v_72.zw, bvec2((((v_71 & 15u) >> 2u) == 2u)))));
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec2 arg_2 = vec2(1.0f);
  tint_ExternalTextureParams v_73 = v_46(0u);
  vec4 res = tint_TextureSampleClampToEdgeMultiplanarExternal(v_73, arg_2);
  return res;
}
void main() {
  v_1.inner = textureSampleBaseClampToEdge_7c04e6();
}
//
// compute_main
//
#version 310 es


struct tint_TransferFunctionParams {
  uint mode;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  float G;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_TransferFunctionParams srcTransferFunction;
  tint_TransferFunctionParams dstTransferFunction;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 apparentSize;
  vec2 plane1CoordFactor;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v_1;
layout(binding = 4, std140)
uniform arg_0_params_block_1_ubo {
  uvec4 inner[17];
} v_2;
uniform highp sampler2D arg_0_plane0_arg_1;
uniform highp sampler2D arg_0_plane1_arg_1;
vec3 tint_ApplyGammaTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  vec3 v_3 = vec3(params.G);
  vec3 v_4 = abs(v);
  vec3 v_5 = sign(v);
  return mix((v_5 * (pow(((params.A * v_4) + params.B), v_3) + params.E)), (v_5 * ((params.C * v_4) + params.F)), lessThan(v_4, vec3(params.D)));
}
float tint_ApplyHLGSingleChannel(float v, tint_TransferFunctionParams params) {
  if ((v <= params.D)) {
    return ((v * v) / params.E);
  } else {
    return ((params.B + exp(((v - params.C) / params.A))) / params.F);
  }
  /* unreachable */
  return 0.0f;
}
vec3 tint_ApplyHLGTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  float v_6 = tint_ApplyHLGSingleChannel(v.x, params);
  float v_7 = tint_ApplyHLGSingleChannel(v.y, params);
  return vec3(v_6, v_7, tint_ApplyHLGSingleChannel(v.z, params));
}
vec3 tint_ApplyPQTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  vec3 v_8 = vec3(params.C);
  vec3 v_9 = vec3(params.D);
  vec3 v_10 = vec3(params.E);
  vec3 v_11 = vec3(params.A);
  vec3 v_12 = pow(clamp(v, vec3(0.0f), vec3(1.0f)), (vec3(1.0f) / vec3(params.B)));
  return pow((max((v_12 - v_8), vec3(0.0f)) / (v_9 - (v_10 * v_12))), (vec3(1.0f) / v_11));
}
vec3 tint_ApplySrcTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  if ((params.mode == 0u)) {
    return tint_ApplyGammaTransferFunction(v, params);
  } else {
    if ((params.mode == 1u)) {
      return tint_ApplyHLGTransferFunction(v, params);
    } else {
      return tint_ApplyPQTransferFunction(v, params);
    }
    /* unreachable */
    return vec3(0.0f);
  }
  /* unreachable */
  return vec3(0.0f);
}
vec4 tint_TextureSampleClampToEdgeMultiplanarExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_13 = (params.sampleTransform * vec3(coords, 1.0f));
  vec2 v_14 = clamp(v_13, params.samplePlane0RectMin, params.samplePlane0RectMax);
  vec3 v_15 = vec3(0.0f);
  float v_16 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_17 = textureLod(arg_0_plane0_arg_1, v_14, 0.0f);
    v_15 = v_17.xyz;
    v_16 = v_17.w;
  } else {
    v_15 = (vec4(textureLod(arg_0_plane0_arg_1, v_14, 0.0f).x, textureLod(arg_0_plane1_arg_1, clamp(v_13, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_16 = 1.0f;
  }
  vec3 v_18 = v_15;
  vec3 v_19 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_19 = tint_ApplyGammaTransferFunction((params.gamutConversionMatrix * tint_ApplySrcTransferFunction(v_18, params.srcTransferFunction)), params.dstTransferFunction);
  } else {
    v_19 = v_18;
  }
  return vec4(v_19, v_16);
}
mat3x2 v_20(uint start_byte_offset) {
  uvec4 v_21 = v_2.inner[(start_byte_offset / 16u)];
  vec2 v_22 = uintBitsToFloat(mix(v_21.xy, v_21.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_23 = (8u + start_byte_offset);
  uvec4 v_24 = v_2.inner[(v_23 / 16u)];
  vec2 v_25 = uintBitsToFloat(mix(v_24.xy, v_24.zw, bvec2((((v_23 & 15u) >> 2u) == 2u))));
  uint v_26 = (16u + start_byte_offset);
  uvec4 v_27 = v_2.inner[(v_26 / 16u)];
  return mat3x2(v_22, v_25, uintBitsToFloat(mix(v_27.xy, v_27.zw, bvec2((((v_26 & 15u) >> 2u) == 2u)))));
}
mat3 v_28(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_TransferFunctionParams v_29(uint start_byte_offset) {
  uvec4 v_30 = v_2.inner[(start_byte_offset / 16u)];
  uint v_31 = (4u + start_byte_offset);
  uvec4 v_32 = v_2.inner[(v_31 / 16u)];
  uint v_33 = (8u + start_byte_offset);
  uvec4 v_34 = v_2.inner[(v_33 / 16u)];
  uint v_35 = (12u + start_byte_offset);
  uvec4 v_36 = v_2.inner[(v_35 / 16u)];
  uint v_37 = (16u + start_byte_offset);
  uvec4 v_38 = v_2.inner[(v_37 / 16u)];
  uint v_39 = (20u + start_byte_offset);
  uvec4 v_40 = v_2.inner[(v_39 / 16u)];
  uint v_41 = (24u + start_byte_offset);
  uvec4 v_42 = v_2.inner[(v_41 / 16u)];
  uint v_43 = (28u + start_byte_offset);
  uvec4 v_44 = v_2.inner[(v_43 / 16u)];
  return tint_TransferFunctionParams(v_30[((start_byte_offset & 15u) >> 2u)], uintBitsToFloat(v_32[((v_31 & 15u) >> 2u)]), uintBitsToFloat(v_34[((v_33 & 15u) >> 2u)]), uintBitsToFloat(v_36[((v_35 & 15u) >> 2u)]), uintBitsToFloat(v_38[((v_37 & 15u) >> 2u)]), uintBitsToFloat(v_40[((v_39 & 15u) >> 2u)]), uintBitsToFloat(v_42[((v_41 & 15u) >> 2u)]), uintBitsToFloat(v_44[((v_43 & 15u) >> 2u)]));
}
mat3x4 v_45(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_46(uint start_byte_offset) {
  uvec4 v_47 = v_2.inner[(start_byte_offset / 16u)];
  uint v_48 = (4u + start_byte_offset);
  uvec4 v_49 = v_2.inner[(v_48 / 16u)];
  mat3x4 v_50 = v_45((16u + start_byte_offset));
  tint_TransferFunctionParams v_51 = v_29((64u + start_byte_offset));
  tint_TransferFunctionParams v_52 = v_29((96u + start_byte_offset));
  mat3 v_53 = v_28((128u + start_byte_offset));
  mat3x2 v_54 = v_20((176u + start_byte_offset));
  mat3x2 v_55 = v_20((200u + start_byte_offset));
  uint v_56 = (224u + start_byte_offset);
  uvec4 v_57 = v_2.inner[(v_56 / 16u)];
  vec2 v_58 = uintBitsToFloat(mix(v_57.xy, v_57.zw, bvec2((((v_56 & 15u) >> 2u) == 2u))));
  uint v_59 = (232u + start_byte_offset);
  uvec4 v_60 = v_2.inner[(v_59 / 16u)];
  vec2 v_61 = uintBitsToFloat(mix(v_60.xy, v_60.zw, bvec2((((v_59 & 15u) >> 2u) == 2u))));
  uint v_62 = (240u + start_byte_offset);
  uvec4 v_63 = v_2.inner[(v_62 / 16u)];
  vec2 v_64 = uintBitsToFloat(mix(v_63.xy, v_63.zw, bvec2((((v_62 & 15u) >> 2u) == 2u))));
  uint v_65 = (248u + start_byte_offset);
  uvec4 v_66 = v_2.inner[(v_65 / 16u)];
  vec2 v_67 = uintBitsToFloat(mix(v_66.xy, v_66.zw, bvec2((((v_65 & 15u) >> 2u) == 2u))));
  uint v_68 = (256u + start_byte_offset);
  uvec4 v_69 = v_2.inner[(v_68 / 16u)];
  uvec2 v_70 = mix(v_69.xy, v_69.zw, bvec2((((v_68 & 15u) >> 2u) == 2u)));
  uint v_71 = (264u + start_byte_offset);
  uvec4 v_72 = v_2.inner[(v_71 / 16u)];
  return tint_ExternalTextureParams(v_47[((start_byte_offset & 15u) >> 2u)], v_49[((v_48 & 15u) >> 2u)], v_50, v_51, v_52, v_53, v_54, v_55, v_58, v_61, v_64, v_67, v_70, uintBitsToFloat(mix(v_72.xy, v_72.zw, bvec2((((v_71 & 15u) >> 2u) == 2u)))));
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec2 arg_2 = vec2(1.0f);
  tint_ExternalTextureParams v_73 = v_46(0u);
  vec4 res = tint_TextureSampleClampToEdgeMultiplanarExternal(v_73, arg_2);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner = textureSampleBaseClampToEdge_7c04e6();
}
//
// vertex_main
//
#version 310 es


struct tint_TransferFunctionParams {
  uint mode;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  float G;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_TransferFunctionParams srcTransferFunction;
  tint_TransferFunctionParams dstTransferFunction;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 apparentSize;
  vec2 plane1CoordFactor;
};

struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 3, std140)
uniform v_arg_0_params_block_ubo {
  uvec4 inner[17];
} v_1;
uniform highp sampler2D v_arg_0_plane0_arg_1;
uniform highp sampler2D v_arg_0_plane1_arg_1;
layout(location = 0) flat out vec4 tint_interstage_location0;
vec3 tint_ApplyGammaTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  vec3 v_2 = vec3(params.G);
  vec3 v_3 = abs(v);
  vec3 v_4 = sign(v);
  return mix((v_4 * (pow(((params.A * v_3) + params.B), v_2) + params.E)), (v_4 * ((params.C * v_3) + params.F)), lessThan(v_3, vec3(params.D)));
}
float tint_ApplyHLGSingleChannel(float v, tint_TransferFunctionParams params) {
  if ((v <= params.D)) {
    return ((v * v) / params.E);
  } else {
    return ((params.B + exp(((v - params.C) / params.A))) / params.F);
  }
  /* unreachable */
  return 0.0f;
}
vec3 tint_ApplyHLGTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  float v_5 = tint_ApplyHLGSingleChannel(v.x, params);
  float v_6 = tint_ApplyHLGSingleChannel(v.y, params);
  return vec3(v_5, v_6, tint_ApplyHLGSingleChannel(v.z, params));
}
vec3 tint_ApplyPQTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  vec3 v_7 = vec3(params.C);
  vec3 v_8 = vec3(params.D);
  vec3 v_9 = vec3(params.E);
  vec3 v_10 = vec3(params.A);
  vec3 v_11 = pow(clamp(v, vec3(0.0f), vec3(1.0f)), (vec3(1.0f) / vec3(params.B)));
  return pow((max((v_11 - v_7), vec3(0.0f)) / (v_8 - (v_9 * v_11))), (vec3(1.0f) / v_10));
}
vec3 tint_ApplySrcTransferFunction(vec3 v, tint_TransferFunctionParams params) {
  if ((params.mode == 0u)) {
    return tint_ApplyGammaTransferFunction(v, params);
  } else {
    if ((params.mode == 1u)) {
      return tint_ApplyHLGTransferFunction(v, params);
    } else {
      return tint_ApplyPQTransferFunction(v, params);
    }
    /* unreachable */
    return vec3(0.0f);
  }
  /* unreachable */
  return vec3(0.0f);
}
vec4 tint_TextureSampleClampToEdgeMultiplanarExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_12 = (params.sampleTransform * vec3(coords, 1.0f));
  vec2 v_13 = clamp(v_12, params.samplePlane0RectMin, params.samplePlane0RectMax);
  vec3 v_14 = vec3(0.0f);
  float v_15 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_16 = textureLod(v_arg_0_plane0_arg_1, v_13, 0.0f);
    v_14 = v_16.xyz;
    v_15 = v_16.w;
  } else {
    v_14 = (vec4(textureLod(v_arg_0_plane0_arg_1, v_13, 0.0f).x, textureLod(v_arg_0_plane1_arg_1, clamp(v_12, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_15 = 1.0f;
  }
  vec3 v_17 = v_14;
  vec3 v_18 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_18 = tint_ApplyGammaTransferFunction((params.gamutConversionMatrix * tint_ApplySrcTransferFunction(v_17, params.srcTransferFunction)), params.dstTransferFunction);
  } else {
    v_18 = v_17;
  }
  return vec4(v_18, v_15);
}
mat3x2 v_19(uint start_byte_offset) {
  uvec4 v_20 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_21 = uintBitsToFloat(mix(v_20.xy, v_20.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_22 = (8u + start_byte_offset);
  uvec4 v_23 = v_1.inner[(v_22 / 16u)];
  vec2 v_24 = uintBitsToFloat(mix(v_23.xy, v_23.zw, bvec2((((v_22 & 15u) >> 2u) == 2u))));
  uint v_25 = (16u + start_byte_offset);
  uvec4 v_26 = v_1.inner[(v_25 / 16u)];
  return mat3x2(v_21, v_24, uintBitsToFloat(mix(v_26.xy, v_26.zw, bvec2((((v_25 & 15u) >> 2u) == 2u)))));
}
mat3 v_27(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_TransferFunctionParams v_28(uint start_byte_offset) {
  uvec4 v_29 = v_1.inner[(start_byte_offset / 16u)];
  uint v_30 = (4u + start_byte_offset);
  uvec4 v_31 = v_1.inner[(v_30 / 16u)];
  uint v_32 = (8u + start_byte_offset);
  uvec4 v_33 = v_1.inner[(v_32 / 16u)];
  uint v_34 = (12u + start_byte_offset);
  uvec4 v_35 = v_1.inner[(v_34 / 16u)];
  uint v_36 = (16u + start_byte_offset);
  uvec4 v_37 = v_1.inner[(v_36 / 16u)];
  uint v_38 = (20u + start_byte_offset);
  uvec4 v_39 = v_1.inner[(v_38 / 16u)];
  uint v_40 = (24u + start_byte_offset);
  uvec4 v_41 = v_1.inner[(v_40 / 16u)];
  uint v_42 = (28u + start_byte_offset);
  uvec4 v_43 = v_1.inner[(v_42 / 16u)];
  return tint_TransferFunctionParams(v_29[((start_byte_offset & 15u) >> 2u)], uintBitsToFloat(v_31[((v_30 & 15u) >> 2u)]), uintBitsToFloat(v_33[((v_32 & 15u) >> 2u)]), uintBitsToFloat(v_35[((v_34 & 15u) >> 2u)]), uintBitsToFloat(v_37[((v_36 & 15u) >> 2u)]), uintBitsToFloat(v_39[((v_38 & 15u) >> 2u)]), uintBitsToFloat(v_41[((v_40 & 15u) >> 2u)]), uintBitsToFloat(v_43[((v_42 & 15u) >> 2u)]));
}
mat3x4 v_44(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_45(uint start_byte_offset) {
  uvec4 v_46 = v_1.inner[(start_byte_offset / 16u)];
  uint v_47 = (4u + start_byte_offset);
  uvec4 v_48 = v_1.inner[(v_47 / 16u)];
  mat3x4 v_49 = v_44((16u + start_byte_offset));
  tint_TransferFunctionParams v_50 = v_28((64u + start_byte_offset));
  tint_TransferFunctionParams v_51 = v_28((96u + start_byte_offset));
  mat3 v_52 = v_27((128u + start_byte_offset));
  mat3x2 v_53 = v_19((176u + start_byte_offset));
  mat3x2 v_54 = v_19((200u + start_byte_offset));
  uint v_55 = (224u + start_byte_offset);
  uvec4 v_56 = v_1.inner[(v_55 / 16u)];
  vec2 v_57 = uintBitsToFloat(mix(v_56.xy, v_56.zw, bvec2((((v_55 & 15u) >> 2u) == 2u))));
  uint v_58 = (232u + start_byte_offset);
  uvec4 v_59 = v_1.inner[(v_58 / 16u)];
  vec2 v_60 = uintBitsToFloat(mix(v_59.xy, v_59.zw, bvec2((((v_58 & 15u) >> 2u) == 2u))));
  uint v_61 = (240u + start_byte_offset);
  uvec4 v_62 = v_1.inner[(v_61 / 16u)];
  vec2 v_63 = uintBitsToFloat(mix(v_62.xy, v_62.zw, bvec2((((v_61 & 15u) >> 2u) == 2u))));
  uint v_64 = (248u + start_byte_offset);
  uvec4 v_65 = v_1.inner[(v_64 / 16u)];
  vec2 v_66 = uintBitsToFloat(mix(v_65.xy, v_65.zw, bvec2((((v_64 & 15u) >> 2u) == 2u))));
  uint v_67 = (256u + start_byte_offset);
  uvec4 v_68 = v_1.inner[(v_67 / 16u)];
  uvec2 v_69 = mix(v_68.xy, v_68.zw, bvec2((((v_67 & 15u) >> 2u) == 2u)));
  uint v_70 = (264u + start_byte_offset);
  uvec4 v_71 = v_1.inner[(v_70 / 16u)];
  return tint_ExternalTextureParams(v_46[((start_byte_offset & 15u) >> 2u)], v_48[((v_47 & 15u) >> 2u)], v_49, v_50, v_51, v_52, v_53, v_54, v_57, v_60, v_63, v_66, v_69, uintBitsToFloat(mix(v_71.xy, v_71.zw, bvec2((((v_70 & 15u) >> 2u) == 2u)))));
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec2 arg_2 = vec2(1.0f);
  tint_ExternalTextureParams v_72 = v_45(0u);
  vec4 res = tint_TextureSampleClampToEdgeMultiplanarExternal(v_72, arg_2);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_73 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_73.pos = vec4(0.0f);
  v_73.prevent_dce = textureSampleBaseClampToEdge_7c04e6();
  return v_73;
}
void main() {
  VertexOutput v_74 = vertex_main_inner();
  gl_Position = vec4(v_74.pos.x, -(v_74.pos.y), ((2.0f * v_74.pos.z) - v_74.pos.w), v_74.pos.w);
  tint_interstage_location0 = v_74.prevent_dce;
  gl_PointSize = 1.0f;
}
