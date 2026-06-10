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
  vec4 ootfParam;
};

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v_1;
layout(binding = 3, std140)
uniform f_arg_0_params_block_ubo {
  uvec4 inner[18];
} v_2;
uniform highp sampler2D f_arg_0_plane0;
uniform highp sampler2D f_arg_0_plane1;
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
vec4 tint_TextureLoadMultiplanarExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_13 = round((params.loadTransform * vec3(vec2(min(coords, params.apparentSize)), 1.0f)));
  uvec2 v_14 = uvec2(v_13);
  vec3 v_15 = vec3(0.0f);
  float v_16 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_17 = ivec2(v_14);
    vec4 v_18 = texelFetch(f_arg_0_plane0, v_17, int(0u));
    v_15 = v_18.xyz;
    v_16 = v_18.w;
  } else {
    ivec2 v_19 = ivec2(v_14);
    float v_20 = texelFetch(f_arg_0_plane0, v_19, int(0u)).x;
    ivec2 v_21 = ivec2(uvec2((v_13 * params.plane1CoordFactor)));
    v_15 = (vec4(v_20, texelFetch(f_arg_0_plane1, v_21, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_16 = 1.0f;
  }
  vec3 v_22 = v_15;
  vec3 v_23 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    vec3 v_24 = tint_ApplySrcTransferFunction(v_22, params.srcTransferFunction);
    vec3 v_25 = vec3(0.0f);
    if ((params.ootfParam.w != 0.0f)) {
      float v_26 = dot(params.ootfParam.xyz, v_24);
      v_25 = (v_24 * (sign(v_26) * pow(abs(v_26), params.ootfParam.w)));
    } else {
      v_25 = v_24;
    }
    v_23 = tint_ApplyGammaTransferFunction((params.gamutConversionMatrix * v_25), params.dstTransferFunction);
  } else {
    v_23 = v_22;
  }
  return vec4(v_23, v_16);
}
mat3x2 v_27(uint start_byte_offset) {
  uvec4 v_28 = v_2.inner[(start_byte_offset / 16u)];
  vec2 v_29 = uintBitsToFloat(mix(v_28.xy, v_28.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_30 = (8u + start_byte_offset);
  uvec4 v_31 = v_2.inner[(v_30 / 16u)];
  vec2 v_32 = uintBitsToFloat(mix(v_31.xy, v_31.zw, bvec2((((v_30 & 15u) >> 2u) == 2u))));
  uint v_33 = (16u + start_byte_offset);
  uvec4 v_34 = v_2.inner[(v_33 / 16u)];
  return mat3x2(v_29, v_32, uintBitsToFloat(mix(v_34.xy, v_34.zw, bvec2((((v_33 & 15u) >> 2u) == 2u)))));
}
mat3 v_35(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_TransferFunctionParams v_36(uint start_byte_offset) {
  uvec4 v_37 = v_2.inner[(start_byte_offset / 16u)];
  uint v_38 = (4u + start_byte_offset);
  uvec4 v_39 = v_2.inner[(v_38 / 16u)];
  uint v_40 = (8u + start_byte_offset);
  uvec4 v_41 = v_2.inner[(v_40 / 16u)];
  uint v_42 = (12u + start_byte_offset);
  uvec4 v_43 = v_2.inner[(v_42 / 16u)];
  uint v_44 = (16u + start_byte_offset);
  uvec4 v_45 = v_2.inner[(v_44 / 16u)];
  uint v_46 = (20u + start_byte_offset);
  uvec4 v_47 = v_2.inner[(v_46 / 16u)];
  uint v_48 = (24u + start_byte_offset);
  uvec4 v_49 = v_2.inner[(v_48 / 16u)];
  uint v_50 = (28u + start_byte_offset);
  uvec4 v_51 = v_2.inner[(v_50 / 16u)];
  return tint_TransferFunctionParams(v_37[((start_byte_offset & 15u) >> 2u)], uintBitsToFloat(v_39[((v_38 & 15u) >> 2u)]), uintBitsToFloat(v_41[((v_40 & 15u) >> 2u)]), uintBitsToFloat(v_43[((v_42 & 15u) >> 2u)]), uintBitsToFloat(v_45[((v_44 & 15u) >> 2u)]), uintBitsToFloat(v_47[((v_46 & 15u) >> 2u)]), uintBitsToFloat(v_49[((v_48 & 15u) >> 2u)]), uintBitsToFloat(v_51[((v_50 & 15u) >> 2u)]));
}
mat3x4 v_52(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_53(uint start_byte_offset) {
  uvec4 v_54 = v_2.inner[(start_byte_offset / 16u)];
  uint v_55 = (4u + start_byte_offset);
  uvec4 v_56 = v_2.inner[(v_55 / 16u)];
  mat3x4 v_57 = v_52((16u + start_byte_offset));
  tint_TransferFunctionParams v_58 = v_36((64u + start_byte_offset));
  tint_TransferFunctionParams v_59 = v_36((96u + start_byte_offset));
  mat3 v_60 = v_35((128u + start_byte_offset));
  mat3x2 v_61 = v_27((176u + start_byte_offset));
  mat3x2 v_62 = v_27((200u + start_byte_offset));
  uint v_63 = (224u + start_byte_offset);
  uvec4 v_64 = v_2.inner[(v_63 / 16u)];
  vec2 v_65 = uintBitsToFloat(mix(v_64.xy, v_64.zw, bvec2((((v_63 & 15u) >> 2u) == 2u))));
  uint v_66 = (232u + start_byte_offset);
  uvec4 v_67 = v_2.inner[(v_66 / 16u)];
  vec2 v_68 = uintBitsToFloat(mix(v_67.xy, v_67.zw, bvec2((((v_66 & 15u) >> 2u) == 2u))));
  uint v_69 = (240u + start_byte_offset);
  uvec4 v_70 = v_2.inner[(v_69 / 16u)];
  vec2 v_71 = uintBitsToFloat(mix(v_70.xy, v_70.zw, bvec2((((v_69 & 15u) >> 2u) == 2u))));
  uint v_72 = (248u + start_byte_offset);
  uvec4 v_73 = v_2.inner[(v_72 / 16u)];
  vec2 v_74 = uintBitsToFloat(mix(v_73.xy, v_73.zw, bvec2((((v_72 & 15u) >> 2u) == 2u))));
  uint v_75 = (256u + start_byte_offset);
  uvec4 v_76 = v_2.inner[(v_75 / 16u)];
  uvec2 v_77 = mix(v_76.xy, v_76.zw, bvec2((((v_75 & 15u) >> 2u) == 2u)));
  uint v_78 = (264u + start_byte_offset);
  uvec4 v_79 = v_2.inner[(v_78 / 16u)];
  vec2 v_80 = uintBitsToFloat(mix(v_79.xy, v_79.zw, bvec2((((v_78 & 15u) >> 2u) == 2u))));
  return tint_ExternalTextureParams(v_54[((start_byte_offset & 15u) >> 2u)], v_56[((v_55 & 15u) >> 2u)], v_57, v_58, v_59, v_60, v_61, v_62, v_65, v_68, v_71, v_74, v_77, v_80, uintBitsToFloat(v_2.inner[((272u + start_byte_offset) / 16u)]));
}
vec4 textureLoad_8acf41() {
  ivec2 arg_1 = ivec2(1);
  tint_ExternalTextureParams v_81 = v_53(0u);
  vec4 res = tint_TextureLoadMultiplanarExternal(v_81, min(uvec2(arg_1), ((v_81.apparentSize + uvec2(1u)) - uvec2(1u))));
  return res;
}
void main() {
  v_1.inner = textureLoad_8acf41();
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
  vec4 ootfParam;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v_1;
layout(binding = 3, std140)
uniform arg_0_params_block_1_ubo {
  uvec4 inner[18];
} v_2;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
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
vec4 tint_TextureLoadMultiplanarExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_13 = round((params.loadTransform * vec3(vec2(min(coords, params.apparentSize)), 1.0f)));
  uvec2 v_14 = uvec2(v_13);
  vec3 v_15 = vec3(0.0f);
  float v_16 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_17 = ivec2(v_14);
    vec4 v_18 = texelFetch(arg_0_plane0, v_17, int(0u));
    v_15 = v_18.xyz;
    v_16 = v_18.w;
  } else {
    ivec2 v_19 = ivec2(v_14);
    float v_20 = texelFetch(arg_0_plane0, v_19, int(0u)).x;
    ivec2 v_21 = ivec2(uvec2((v_13 * params.plane1CoordFactor)));
    v_15 = (vec4(v_20, texelFetch(arg_0_plane1, v_21, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_16 = 1.0f;
  }
  vec3 v_22 = v_15;
  vec3 v_23 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    vec3 v_24 = tint_ApplySrcTransferFunction(v_22, params.srcTransferFunction);
    vec3 v_25 = vec3(0.0f);
    if ((params.ootfParam.w != 0.0f)) {
      float v_26 = dot(params.ootfParam.xyz, v_24);
      v_25 = (v_24 * (sign(v_26) * pow(abs(v_26), params.ootfParam.w)));
    } else {
      v_25 = v_24;
    }
    v_23 = tint_ApplyGammaTransferFunction((params.gamutConversionMatrix * v_25), params.dstTransferFunction);
  } else {
    v_23 = v_22;
  }
  return vec4(v_23, v_16);
}
mat3x2 v_27(uint start_byte_offset) {
  uvec4 v_28 = v_2.inner[(start_byte_offset / 16u)];
  vec2 v_29 = uintBitsToFloat(mix(v_28.xy, v_28.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_30 = (8u + start_byte_offset);
  uvec4 v_31 = v_2.inner[(v_30 / 16u)];
  vec2 v_32 = uintBitsToFloat(mix(v_31.xy, v_31.zw, bvec2((((v_30 & 15u) >> 2u) == 2u))));
  uint v_33 = (16u + start_byte_offset);
  uvec4 v_34 = v_2.inner[(v_33 / 16u)];
  return mat3x2(v_29, v_32, uintBitsToFloat(mix(v_34.xy, v_34.zw, bvec2((((v_33 & 15u) >> 2u) == 2u)))));
}
mat3 v_35(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_TransferFunctionParams v_36(uint start_byte_offset) {
  uvec4 v_37 = v_2.inner[(start_byte_offset / 16u)];
  uint v_38 = (4u + start_byte_offset);
  uvec4 v_39 = v_2.inner[(v_38 / 16u)];
  uint v_40 = (8u + start_byte_offset);
  uvec4 v_41 = v_2.inner[(v_40 / 16u)];
  uint v_42 = (12u + start_byte_offset);
  uvec4 v_43 = v_2.inner[(v_42 / 16u)];
  uint v_44 = (16u + start_byte_offset);
  uvec4 v_45 = v_2.inner[(v_44 / 16u)];
  uint v_46 = (20u + start_byte_offset);
  uvec4 v_47 = v_2.inner[(v_46 / 16u)];
  uint v_48 = (24u + start_byte_offset);
  uvec4 v_49 = v_2.inner[(v_48 / 16u)];
  uint v_50 = (28u + start_byte_offset);
  uvec4 v_51 = v_2.inner[(v_50 / 16u)];
  return tint_TransferFunctionParams(v_37[((start_byte_offset & 15u) >> 2u)], uintBitsToFloat(v_39[((v_38 & 15u) >> 2u)]), uintBitsToFloat(v_41[((v_40 & 15u) >> 2u)]), uintBitsToFloat(v_43[((v_42 & 15u) >> 2u)]), uintBitsToFloat(v_45[((v_44 & 15u) >> 2u)]), uintBitsToFloat(v_47[((v_46 & 15u) >> 2u)]), uintBitsToFloat(v_49[((v_48 & 15u) >> 2u)]), uintBitsToFloat(v_51[((v_50 & 15u) >> 2u)]));
}
mat3x4 v_52(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_53(uint start_byte_offset) {
  uvec4 v_54 = v_2.inner[(start_byte_offset / 16u)];
  uint v_55 = (4u + start_byte_offset);
  uvec4 v_56 = v_2.inner[(v_55 / 16u)];
  mat3x4 v_57 = v_52((16u + start_byte_offset));
  tint_TransferFunctionParams v_58 = v_36((64u + start_byte_offset));
  tint_TransferFunctionParams v_59 = v_36((96u + start_byte_offset));
  mat3 v_60 = v_35((128u + start_byte_offset));
  mat3x2 v_61 = v_27((176u + start_byte_offset));
  mat3x2 v_62 = v_27((200u + start_byte_offset));
  uint v_63 = (224u + start_byte_offset);
  uvec4 v_64 = v_2.inner[(v_63 / 16u)];
  vec2 v_65 = uintBitsToFloat(mix(v_64.xy, v_64.zw, bvec2((((v_63 & 15u) >> 2u) == 2u))));
  uint v_66 = (232u + start_byte_offset);
  uvec4 v_67 = v_2.inner[(v_66 / 16u)];
  vec2 v_68 = uintBitsToFloat(mix(v_67.xy, v_67.zw, bvec2((((v_66 & 15u) >> 2u) == 2u))));
  uint v_69 = (240u + start_byte_offset);
  uvec4 v_70 = v_2.inner[(v_69 / 16u)];
  vec2 v_71 = uintBitsToFloat(mix(v_70.xy, v_70.zw, bvec2((((v_69 & 15u) >> 2u) == 2u))));
  uint v_72 = (248u + start_byte_offset);
  uvec4 v_73 = v_2.inner[(v_72 / 16u)];
  vec2 v_74 = uintBitsToFloat(mix(v_73.xy, v_73.zw, bvec2((((v_72 & 15u) >> 2u) == 2u))));
  uint v_75 = (256u + start_byte_offset);
  uvec4 v_76 = v_2.inner[(v_75 / 16u)];
  uvec2 v_77 = mix(v_76.xy, v_76.zw, bvec2((((v_75 & 15u) >> 2u) == 2u)));
  uint v_78 = (264u + start_byte_offset);
  uvec4 v_79 = v_2.inner[(v_78 / 16u)];
  vec2 v_80 = uintBitsToFloat(mix(v_79.xy, v_79.zw, bvec2((((v_78 & 15u) >> 2u) == 2u))));
  return tint_ExternalTextureParams(v_54[((start_byte_offset & 15u) >> 2u)], v_56[((v_55 & 15u) >> 2u)], v_57, v_58, v_59, v_60, v_61, v_62, v_65, v_68, v_71, v_74, v_77, v_80, uintBitsToFloat(v_2.inner[((272u + start_byte_offset) / 16u)]));
}
vec4 textureLoad_8acf41() {
  ivec2 arg_1 = ivec2(1);
  tint_ExternalTextureParams v_81 = v_53(0u);
  vec4 res = tint_TextureLoadMultiplanarExternal(v_81, min(uvec2(arg_1), ((v_81.apparentSize + uvec2(1u)) - uvec2(1u))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner = textureLoad_8acf41();
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
  vec4 ootfParam;
};

struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 2, std140)
uniform v_arg_0_params_block_ubo {
  uvec4 inner[18];
} v_1;
uniform highp sampler2D v_arg_0_plane0;
uniform highp sampler2D v_arg_0_plane1;
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
vec4 tint_TextureLoadMultiplanarExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_12 = round((params.loadTransform * vec3(vec2(min(coords, params.apparentSize)), 1.0f)));
  uvec2 v_13 = uvec2(v_12);
  vec3 v_14 = vec3(0.0f);
  float v_15 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_16 = ivec2(v_13);
    vec4 v_17 = texelFetch(v_arg_0_plane0, v_16, int(0u));
    v_14 = v_17.xyz;
    v_15 = v_17.w;
  } else {
    ivec2 v_18 = ivec2(v_13);
    float v_19 = texelFetch(v_arg_0_plane0, v_18, int(0u)).x;
    ivec2 v_20 = ivec2(uvec2((v_12 * params.plane1CoordFactor)));
    v_14 = (vec4(v_19, texelFetch(v_arg_0_plane1, v_20, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_15 = 1.0f;
  }
  vec3 v_21 = v_14;
  vec3 v_22 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    vec3 v_23 = tint_ApplySrcTransferFunction(v_21, params.srcTransferFunction);
    vec3 v_24 = vec3(0.0f);
    if ((params.ootfParam.w != 0.0f)) {
      float v_25 = dot(params.ootfParam.xyz, v_23);
      v_24 = (v_23 * (sign(v_25) * pow(abs(v_25), params.ootfParam.w)));
    } else {
      v_24 = v_23;
    }
    v_22 = tint_ApplyGammaTransferFunction((params.gamutConversionMatrix * v_24), params.dstTransferFunction);
  } else {
    v_22 = v_21;
  }
  return vec4(v_22, v_15);
}
mat3x2 v_26(uint start_byte_offset) {
  uvec4 v_27 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_28 = uintBitsToFloat(mix(v_27.xy, v_27.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_29 = (8u + start_byte_offset);
  uvec4 v_30 = v_1.inner[(v_29 / 16u)];
  vec2 v_31 = uintBitsToFloat(mix(v_30.xy, v_30.zw, bvec2((((v_29 & 15u) >> 2u) == 2u))));
  uint v_32 = (16u + start_byte_offset);
  uvec4 v_33 = v_1.inner[(v_32 / 16u)];
  return mat3x2(v_28, v_31, uintBitsToFloat(mix(v_33.xy, v_33.zw, bvec2((((v_32 & 15u) >> 2u) == 2u)))));
}
mat3 v_34(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_TransferFunctionParams v_35(uint start_byte_offset) {
  uvec4 v_36 = v_1.inner[(start_byte_offset / 16u)];
  uint v_37 = (4u + start_byte_offset);
  uvec4 v_38 = v_1.inner[(v_37 / 16u)];
  uint v_39 = (8u + start_byte_offset);
  uvec4 v_40 = v_1.inner[(v_39 / 16u)];
  uint v_41 = (12u + start_byte_offset);
  uvec4 v_42 = v_1.inner[(v_41 / 16u)];
  uint v_43 = (16u + start_byte_offset);
  uvec4 v_44 = v_1.inner[(v_43 / 16u)];
  uint v_45 = (20u + start_byte_offset);
  uvec4 v_46 = v_1.inner[(v_45 / 16u)];
  uint v_47 = (24u + start_byte_offset);
  uvec4 v_48 = v_1.inner[(v_47 / 16u)];
  uint v_49 = (28u + start_byte_offset);
  uvec4 v_50 = v_1.inner[(v_49 / 16u)];
  return tint_TransferFunctionParams(v_36[((start_byte_offset & 15u) >> 2u)], uintBitsToFloat(v_38[((v_37 & 15u) >> 2u)]), uintBitsToFloat(v_40[((v_39 & 15u) >> 2u)]), uintBitsToFloat(v_42[((v_41 & 15u) >> 2u)]), uintBitsToFloat(v_44[((v_43 & 15u) >> 2u)]), uintBitsToFloat(v_46[((v_45 & 15u) >> 2u)]), uintBitsToFloat(v_48[((v_47 & 15u) >> 2u)]), uintBitsToFloat(v_50[((v_49 & 15u) >> 2u)]));
}
mat3x4 v_51(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_52(uint start_byte_offset) {
  uvec4 v_53 = v_1.inner[(start_byte_offset / 16u)];
  uint v_54 = (4u + start_byte_offset);
  uvec4 v_55 = v_1.inner[(v_54 / 16u)];
  mat3x4 v_56 = v_51((16u + start_byte_offset));
  tint_TransferFunctionParams v_57 = v_35((64u + start_byte_offset));
  tint_TransferFunctionParams v_58 = v_35((96u + start_byte_offset));
  mat3 v_59 = v_34((128u + start_byte_offset));
  mat3x2 v_60 = v_26((176u + start_byte_offset));
  mat3x2 v_61 = v_26((200u + start_byte_offset));
  uint v_62 = (224u + start_byte_offset);
  uvec4 v_63 = v_1.inner[(v_62 / 16u)];
  vec2 v_64 = uintBitsToFloat(mix(v_63.xy, v_63.zw, bvec2((((v_62 & 15u) >> 2u) == 2u))));
  uint v_65 = (232u + start_byte_offset);
  uvec4 v_66 = v_1.inner[(v_65 / 16u)];
  vec2 v_67 = uintBitsToFloat(mix(v_66.xy, v_66.zw, bvec2((((v_65 & 15u) >> 2u) == 2u))));
  uint v_68 = (240u + start_byte_offset);
  uvec4 v_69 = v_1.inner[(v_68 / 16u)];
  vec2 v_70 = uintBitsToFloat(mix(v_69.xy, v_69.zw, bvec2((((v_68 & 15u) >> 2u) == 2u))));
  uint v_71 = (248u + start_byte_offset);
  uvec4 v_72 = v_1.inner[(v_71 / 16u)];
  vec2 v_73 = uintBitsToFloat(mix(v_72.xy, v_72.zw, bvec2((((v_71 & 15u) >> 2u) == 2u))));
  uint v_74 = (256u + start_byte_offset);
  uvec4 v_75 = v_1.inner[(v_74 / 16u)];
  uvec2 v_76 = mix(v_75.xy, v_75.zw, bvec2((((v_74 & 15u) >> 2u) == 2u)));
  uint v_77 = (264u + start_byte_offset);
  uvec4 v_78 = v_1.inner[(v_77 / 16u)];
  vec2 v_79 = uintBitsToFloat(mix(v_78.xy, v_78.zw, bvec2((((v_77 & 15u) >> 2u) == 2u))));
  return tint_ExternalTextureParams(v_53[((start_byte_offset & 15u) >> 2u)], v_55[((v_54 & 15u) >> 2u)], v_56, v_57, v_58, v_59, v_60, v_61, v_64, v_67, v_70, v_73, v_76, v_79, uintBitsToFloat(v_1.inner[((272u + start_byte_offset) / 16u)]));
}
vec4 textureLoad_8acf41() {
  ivec2 arg_1 = ivec2(1);
  tint_ExternalTextureParams v_80 = v_52(0u);
  vec4 res = tint_TextureLoadMultiplanarExternal(v_80, min(uvec2(arg_1), ((v_80.apparentSize + uvec2(1u)) - uvec2(1u))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_81 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_81.pos = vec4(0.0f);
  v_81.prevent_dce = textureLoad_8acf41();
  return v_81;
}
void main() {
  VertexOutput v_82 = vertex_main_inner();
  gl_Position = vec4(v_82.pos.x, -(v_82.pos.y), ((2.0f * v_82.pos.z) - v_82.pos.w), v_82.pos.w);
  tint_interstage_location0 = v_82.prevent_dce;
  gl_PointSize = 1.0f;
}
