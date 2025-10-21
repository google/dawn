//
// vertex_main
//
#version 310 es


struct tint_GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
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

layout(binding = 2, std140)
uniform v_arg_0_params_block_ubo {
  uvec4 inner[17];
} v_1;
uniform highp sampler2D v_arg_0_plane0;
uniform highp sampler2D v_arg_0_plane1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_2 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_2) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureLoadExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_3 = round((params.loadTransform * vec3(vec2(min(coords, params.apparentSize)), 1.0f)));
  uvec2 v_4 = uvec2(v_3);
  vec3 v_5 = vec3(0.0f);
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_7 = ivec2(v_4);
    vec4 v_8 = texelFetch(v_arg_0_plane0, v_7, int(0u));
    v_5 = v_8.xyz;
    v_6 = v_8.w;
  } else {
    ivec2 v_9 = ivec2(v_4);
    float v_10 = texelFetch(v_arg_0_plane0, v_9, int(0u)).x;
    ivec2 v_11 = ivec2(uvec2((v_3 * params.plane1CoordFactor)));
    v_5 = (vec4(v_10, texelFetch(v_arg_0_plane1, v_11, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_6 = 1.0f;
  }
  vec3 v_12 = v_5;
  vec3 v_13 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_13 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_12, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_13 = v_12;
  }
  return vec4(v_13, v_6);
}
vec4 textureLoad2d(tint_ExternalTextureParams texture_params, ivec2 coords) {
  return tint_TextureLoadExternal(texture_params, min(uvec2(coords), ((texture_params.apparentSize + uvec2(1u)) - uvec2(1u))));
}
mat3x2 v_14(uint start_byte_offset) {
  uvec4 v_15 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_16 = uintBitsToFloat(mix(v_15.xy, v_15.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_17 = v_1.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_18 = uintBitsToFloat(mix(v_17.xy, v_17.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_19 = v_1.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_16, v_18, uintBitsToFloat(mix(v_19.xy, v_19.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat3 v_20(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_GammaTransferParams v_21(uint start_byte_offset) {
  uvec4 v_22 = v_1.inner[(start_byte_offset / 16u)];
  uvec4 v_23 = v_1.inner[((4u + start_byte_offset) / 16u)];
  uvec4 v_24 = v_1.inner[((8u + start_byte_offset) / 16u)];
  uvec4 v_25 = v_1.inner[((12u + start_byte_offset) / 16u)];
  uvec4 v_26 = v_1.inner[((16u + start_byte_offset) / 16u)];
  uvec4 v_27 = v_1.inner[((20u + start_byte_offset) / 16u)];
  uvec4 v_28 = v_1.inner[((24u + start_byte_offset) / 16u)];
  uvec4 v_29 = v_1.inner[((28u + start_byte_offset) / 16u)];
  return tint_GammaTransferParams(uintBitsToFloat(v_22[((start_byte_offset % 16u) / 4u)]), uintBitsToFloat(v_23[(((4u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_24[(((8u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_25[(((12u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_26[(((16u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_27[(((20u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_28[(((24u + start_byte_offset) % 16u) / 4u)]), v_29[(((28u + start_byte_offset) % 16u) / 4u)]);
}
mat3x4 v_30(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_31(uint start_byte_offset) {
  uvec4 v_32 = v_1.inner[(start_byte_offset / 16u)];
  uvec4 v_33 = v_1.inner[((4u + start_byte_offset) / 16u)];
  mat3x4 v_34 = v_30((16u + start_byte_offset));
  tint_GammaTransferParams v_35 = v_21((64u + start_byte_offset));
  tint_GammaTransferParams v_36 = v_21((96u + start_byte_offset));
  mat3 v_37 = v_20((128u + start_byte_offset));
  mat3x2 v_38 = v_14((176u + start_byte_offset));
  mat3x2 v_39 = v_14((200u + start_byte_offset));
  uvec4 v_40 = v_1.inner[((224u + start_byte_offset) / 16u)];
  vec2 v_41 = uintBitsToFloat(mix(v_40.xy, v_40.zw, bvec2(((((224u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_42 = v_1.inner[((232u + start_byte_offset) / 16u)];
  vec2 v_43 = uintBitsToFloat(mix(v_42.xy, v_42.zw, bvec2(((((232u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_44 = v_1.inner[((240u + start_byte_offset) / 16u)];
  vec2 v_45 = uintBitsToFloat(mix(v_44.xy, v_44.zw, bvec2(((((240u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_46 = v_1.inner[((248u + start_byte_offset) / 16u)];
  vec2 v_47 = uintBitsToFloat(mix(v_46.xy, v_46.zw, bvec2(((((248u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_48 = v_1.inner[((256u + start_byte_offset) / 16u)];
  uvec2 v_49 = mix(v_48.xy, v_48.zw, bvec2(((((256u + start_byte_offset) % 16u) / 4u) == 2u)));
  uvec4 v_50 = v_1.inner[((264u + start_byte_offset) / 16u)];
  return tint_ExternalTextureParams(v_32[((start_byte_offset % 16u) / 4u)], v_33[(((4u + start_byte_offset) % 16u) / 4u)], v_34, v_35, v_36, v_37, v_38, v_39, v_41, v_43, v_45, v_47, v_49, uintBitsToFloat(mix(v_50.xy, v_50.zw, bvec2(((((264u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
void doTextureLoad() {
  vec4 res = textureLoad2d(v_31(0u), ivec2(0));
}
vec4 vertex_main_inner() {
  doTextureLoad();
  return vec4(0.0f);
}
void main() {
  vec4 v_51 = vertex_main_inner();
  gl_Position = vec4(v_51.x, -(v_51.y), ((2.0f * v_51.z) - v_51.w), v_51.w);
  gl_PointSize = 1.0f;
}
//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct tint_GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
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

layout(binding = 2, std140)
uniform f_arg_0_params_block_ubo {
  uvec4 inner[17];
} v_1;
uniform highp sampler2D f_arg_0_plane0;
uniform highp sampler2D f_arg_0_plane1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_2 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_2) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureLoadExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_3 = round((params.loadTransform * vec3(vec2(min(coords, params.apparentSize)), 1.0f)));
  uvec2 v_4 = uvec2(v_3);
  vec3 v_5 = vec3(0.0f);
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_7 = ivec2(v_4);
    vec4 v_8 = texelFetch(f_arg_0_plane0, v_7, int(0u));
    v_5 = v_8.xyz;
    v_6 = v_8.w;
  } else {
    ivec2 v_9 = ivec2(v_4);
    float v_10 = texelFetch(f_arg_0_plane0, v_9, int(0u)).x;
    ivec2 v_11 = ivec2(uvec2((v_3 * params.plane1CoordFactor)));
    v_5 = (vec4(v_10, texelFetch(f_arg_0_plane1, v_11, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_6 = 1.0f;
  }
  vec3 v_12 = v_5;
  vec3 v_13 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_13 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_12, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_13 = v_12;
  }
  return vec4(v_13, v_6);
}
vec4 textureLoad2d(tint_ExternalTextureParams texture_params, ivec2 coords) {
  return tint_TextureLoadExternal(texture_params, min(uvec2(coords), ((texture_params.apparentSize + uvec2(1u)) - uvec2(1u))));
}
mat3x2 v_14(uint start_byte_offset) {
  uvec4 v_15 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_16 = uintBitsToFloat(mix(v_15.xy, v_15.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_17 = v_1.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_18 = uintBitsToFloat(mix(v_17.xy, v_17.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_19 = v_1.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_16, v_18, uintBitsToFloat(mix(v_19.xy, v_19.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat3 v_20(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_GammaTransferParams v_21(uint start_byte_offset) {
  uvec4 v_22 = v_1.inner[(start_byte_offset / 16u)];
  uvec4 v_23 = v_1.inner[((4u + start_byte_offset) / 16u)];
  uvec4 v_24 = v_1.inner[((8u + start_byte_offset) / 16u)];
  uvec4 v_25 = v_1.inner[((12u + start_byte_offset) / 16u)];
  uvec4 v_26 = v_1.inner[((16u + start_byte_offset) / 16u)];
  uvec4 v_27 = v_1.inner[((20u + start_byte_offset) / 16u)];
  uvec4 v_28 = v_1.inner[((24u + start_byte_offset) / 16u)];
  uvec4 v_29 = v_1.inner[((28u + start_byte_offset) / 16u)];
  return tint_GammaTransferParams(uintBitsToFloat(v_22[((start_byte_offset % 16u) / 4u)]), uintBitsToFloat(v_23[(((4u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_24[(((8u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_25[(((12u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_26[(((16u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_27[(((20u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_28[(((24u + start_byte_offset) % 16u) / 4u)]), v_29[(((28u + start_byte_offset) % 16u) / 4u)]);
}
mat3x4 v_30(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_31(uint start_byte_offset) {
  uvec4 v_32 = v_1.inner[(start_byte_offset / 16u)];
  uvec4 v_33 = v_1.inner[((4u + start_byte_offset) / 16u)];
  mat3x4 v_34 = v_30((16u + start_byte_offset));
  tint_GammaTransferParams v_35 = v_21((64u + start_byte_offset));
  tint_GammaTransferParams v_36 = v_21((96u + start_byte_offset));
  mat3 v_37 = v_20((128u + start_byte_offset));
  mat3x2 v_38 = v_14((176u + start_byte_offset));
  mat3x2 v_39 = v_14((200u + start_byte_offset));
  uvec4 v_40 = v_1.inner[((224u + start_byte_offset) / 16u)];
  vec2 v_41 = uintBitsToFloat(mix(v_40.xy, v_40.zw, bvec2(((((224u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_42 = v_1.inner[((232u + start_byte_offset) / 16u)];
  vec2 v_43 = uintBitsToFloat(mix(v_42.xy, v_42.zw, bvec2(((((232u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_44 = v_1.inner[((240u + start_byte_offset) / 16u)];
  vec2 v_45 = uintBitsToFloat(mix(v_44.xy, v_44.zw, bvec2(((((240u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_46 = v_1.inner[((248u + start_byte_offset) / 16u)];
  vec2 v_47 = uintBitsToFloat(mix(v_46.xy, v_46.zw, bvec2(((((248u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_48 = v_1.inner[((256u + start_byte_offset) / 16u)];
  uvec2 v_49 = mix(v_48.xy, v_48.zw, bvec2(((((256u + start_byte_offset) % 16u) / 4u) == 2u)));
  uvec4 v_50 = v_1.inner[((264u + start_byte_offset) / 16u)];
  return tint_ExternalTextureParams(v_32[((start_byte_offset % 16u) / 4u)], v_33[(((4u + start_byte_offset) % 16u) / 4u)], v_34, v_35, v_36, v_37, v_38, v_39, v_41, v_43, v_45, v_47, v_49, uintBitsToFloat(mix(v_50.xy, v_50.zw, bvec2(((((264u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
void doTextureLoad() {
  vec4 res = textureLoad2d(v_31(0u), ivec2(0));
}
void main() {
  doTextureLoad();
}
//
// compute_main
//
#version 310 es


struct tint_GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
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

layout(binding = 2, std140)
uniform arg_0_params_block_1_ubo {
  uvec4 inner[17];
} v_1;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_2 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_2) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureLoadExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_3 = round((params.loadTransform * vec3(vec2(min(coords, params.apparentSize)), 1.0f)));
  uvec2 v_4 = uvec2(v_3);
  vec3 v_5 = vec3(0.0f);
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_7 = ivec2(v_4);
    vec4 v_8 = texelFetch(arg_0_plane0, v_7, int(0u));
    v_5 = v_8.xyz;
    v_6 = v_8.w;
  } else {
    ivec2 v_9 = ivec2(v_4);
    float v_10 = texelFetch(arg_0_plane0, v_9, int(0u)).x;
    ivec2 v_11 = ivec2(uvec2((v_3 * params.plane1CoordFactor)));
    v_5 = (vec4(v_10, texelFetch(arg_0_plane1, v_11, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_6 = 1.0f;
  }
  vec3 v_12 = v_5;
  vec3 v_13 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_13 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_12, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_13 = v_12;
  }
  return vec4(v_13, v_6);
}
vec4 textureLoad2d(tint_ExternalTextureParams texture_params, ivec2 coords) {
  return tint_TextureLoadExternal(texture_params, min(uvec2(coords), ((texture_params.apparentSize + uvec2(1u)) - uvec2(1u))));
}
mat3x2 v_14(uint start_byte_offset) {
  uvec4 v_15 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_16 = uintBitsToFloat(mix(v_15.xy, v_15.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_17 = v_1.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_18 = uintBitsToFloat(mix(v_17.xy, v_17.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_19 = v_1.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_16, v_18, uintBitsToFloat(mix(v_19.xy, v_19.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat3 v_20(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_GammaTransferParams v_21(uint start_byte_offset) {
  uvec4 v_22 = v_1.inner[(start_byte_offset / 16u)];
  uvec4 v_23 = v_1.inner[((4u + start_byte_offset) / 16u)];
  uvec4 v_24 = v_1.inner[((8u + start_byte_offset) / 16u)];
  uvec4 v_25 = v_1.inner[((12u + start_byte_offset) / 16u)];
  uvec4 v_26 = v_1.inner[((16u + start_byte_offset) / 16u)];
  uvec4 v_27 = v_1.inner[((20u + start_byte_offset) / 16u)];
  uvec4 v_28 = v_1.inner[((24u + start_byte_offset) / 16u)];
  uvec4 v_29 = v_1.inner[((28u + start_byte_offset) / 16u)];
  return tint_GammaTransferParams(uintBitsToFloat(v_22[((start_byte_offset % 16u) / 4u)]), uintBitsToFloat(v_23[(((4u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_24[(((8u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_25[(((12u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_26[(((16u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_27[(((20u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_28[(((24u + start_byte_offset) % 16u) / 4u)]), v_29[(((28u + start_byte_offset) % 16u) / 4u)]);
}
mat3x4 v_30(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_31(uint start_byte_offset) {
  uvec4 v_32 = v_1.inner[(start_byte_offset / 16u)];
  uvec4 v_33 = v_1.inner[((4u + start_byte_offset) / 16u)];
  mat3x4 v_34 = v_30((16u + start_byte_offset));
  tint_GammaTransferParams v_35 = v_21((64u + start_byte_offset));
  tint_GammaTransferParams v_36 = v_21((96u + start_byte_offset));
  mat3 v_37 = v_20((128u + start_byte_offset));
  mat3x2 v_38 = v_14((176u + start_byte_offset));
  mat3x2 v_39 = v_14((200u + start_byte_offset));
  uvec4 v_40 = v_1.inner[((224u + start_byte_offset) / 16u)];
  vec2 v_41 = uintBitsToFloat(mix(v_40.xy, v_40.zw, bvec2(((((224u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_42 = v_1.inner[((232u + start_byte_offset) / 16u)];
  vec2 v_43 = uintBitsToFloat(mix(v_42.xy, v_42.zw, bvec2(((((232u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_44 = v_1.inner[((240u + start_byte_offset) / 16u)];
  vec2 v_45 = uintBitsToFloat(mix(v_44.xy, v_44.zw, bvec2(((((240u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_46 = v_1.inner[((248u + start_byte_offset) / 16u)];
  vec2 v_47 = uintBitsToFloat(mix(v_46.xy, v_46.zw, bvec2(((((248u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_48 = v_1.inner[((256u + start_byte_offset) / 16u)];
  uvec2 v_49 = mix(v_48.xy, v_48.zw, bvec2(((((256u + start_byte_offset) % 16u) / 4u) == 2u)));
  uvec4 v_50 = v_1.inner[((264u + start_byte_offset) / 16u)];
  return tint_ExternalTextureParams(v_32[((start_byte_offset % 16u) / 4u)], v_33[(((4u + start_byte_offset) % 16u) / 4u)], v_34, v_35, v_36, v_37, v_38, v_39, v_41, v_43, v_45, v_47, v_49, uintBitsToFloat(mix(v_50.xy, v_50.zw, bvec2(((((264u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
void doTextureLoad() {
  vec4 res = textureLoad2d(v_31(0u), ivec2(0));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  doTextureLoad();
}
