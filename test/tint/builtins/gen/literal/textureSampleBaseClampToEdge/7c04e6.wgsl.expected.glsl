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
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_3 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_3) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureSampleExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_4 = (params.sampleTransform * vec3(coords, 1.0f));
  vec3 v_5 = vec3(0.0f);
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_7 = textureLod(f_arg_0_plane0_arg_1, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f);
    v_5 = v_7.xyz;
    v_6 = v_7.w;
  } else {
    v_5 = (vec4(textureLod(f_arg_0_plane0_arg_1, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f).x, textureLod(f_arg_0_plane1_arg_1, clamp(v_4, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_6 = 1.0f;
  }
  vec3 v_8 = v_5;
  vec3 v_9 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_9 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_8, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_9 = v_8;
  }
  return vec4(v_9, v_6);
}
mat3x2 v_10(uint start_byte_offset) {
  uvec4 v_11 = v_2.inner[(start_byte_offset / 16u)];
  vec2 v_12 = uintBitsToFloat(mix(v_11.xy, v_11.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_13 = v_2.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_14 = uintBitsToFloat(mix(v_13.xy, v_13.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_15 = v_2.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_12, v_14, uintBitsToFloat(mix(v_15.xy, v_15.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat3 v_16(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_GammaTransferParams v_17(uint start_byte_offset) {
  uvec4 v_18 = v_2.inner[(start_byte_offset / 16u)];
  uvec4 v_19 = v_2.inner[((4u + start_byte_offset) / 16u)];
  uvec4 v_20 = v_2.inner[((8u + start_byte_offset) / 16u)];
  uvec4 v_21 = v_2.inner[((12u + start_byte_offset) / 16u)];
  uvec4 v_22 = v_2.inner[((16u + start_byte_offset) / 16u)];
  uvec4 v_23 = v_2.inner[((20u + start_byte_offset) / 16u)];
  uvec4 v_24 = v_2.inner[((24u + start_byte_offset) / 16u)];
  uvec4 v_25 = v_2.inner[((28u + start_byte_offset) / 16u)];
  return tint_GammaTransferParams(uintBitsToFloat(v_18[((start_byte_offset % 16u) / 4u)]), uintBitsToFloat(v_19[(((4u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_20[(((8u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_21[(((12u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_22[(((16u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_23[(((20u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_24[(((24u + start_byte_offset) % 16u) / 4u)]), v_25[(((28u + start_byte_offset) % 16u) / 4u)]);
}
mat3x4 v_26(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_27(uint start_byte_offset) {
  uvec4 v_28 = v_2.inner[(start_byte_offset / 16u)];
  uvec4 v_29 = v_2.inner[((4u + start_byte_offset) / 16u)];
  mat3x4 v_30 = v_26((16u + start_byte_offset));
  tint_GammaTransferParams v_31 = v_17((64u + start_byte_offset));
  tint_GammaTransferParams v_32 = v_17((96u + start_byte_offset));
  mat3 v_33 = v_16((128u + start_byte_offset));
  mat3x2 v_34 = v_10((176u + start_byte_offset));
  mat3x2 v_35 = v_10((200u + start_byte_offset));
  uvec4 v_36 = v_2.inner[((224u + start_byte_offset) / 16u)];
  vec2 v_37 = uintBitsToFloat(mix(v_36.xy, v_36.zw, bvec2(((((224u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_38 = v_2.inner[((232u + start_byte_offset) / 16u)];
  vec2 v_39 = uintBitsToFloat(mix(v_38.xy, v_38.zw, bvec2(((((232u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_40 = v_2.inner[((240u + start_byte_offset) / 16u)];
  vec2 v_41 = uintBitsToFloat(mix(v_40.xy, v_40.zw, bvec2(((((240u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_42 = v_2.inner[((248u + start_byte_offset) / 16u)];
  vec2 v_43 = uintBitsToFloat(mix(v_42.xy, v_42.zw, bvec2(((((248u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_44 = v_2.inner[((256u + start_byte_offset) / 16u)];
  uvec2 v_45 = mix(v_44.xy, v_44.zw, bvec2(((((256u + start_byte_offset) % 16u) / 4u) == 2u)));
  uvec4 v_46 = v_2.inner[((264u + start_byte_offset) / 16u)];
  return tint_ExternalTextureParams(v_28[((start_byte_offset % 16u) / 4u)], v_29[(((4u + start_byte_offset) % 16u) / 4u)], v_30, v_31, v_32, v_33, v_34, v_35, v_37, v_39, v_41, v_43, v_45, uintBitsToFloat(mix(v_46.xy, v_46.zw, bvec2(((((264u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec4 res = tint_TextureSampleExternal(v_27(0u), vec2(1.0f));
  return res;
}
void main() {
  v_1.inner = textureSampleBaseClampToEdge_7c04e6();
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
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_3 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_3) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureSampleExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_4 = (params.sampleTransform * vec3(coords, 1.0f));
  vec3 v_5 = vec3(0.0f);
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_7 = textureLod(arg_0_plane0_arg_1, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f);
    v_5 = v_7.xyz;
    v_6 = v_7.w;
  } else {
    v_5 = (vec4(textureLod(arg_0_plane0_arg_1, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f).x, textureLod(arg_0_plane1_arg_1, clamp(v_4, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_6 = 1.0f;
  }
  vec3 v_8 = v_5;
  vec3 v_9 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_9 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_8, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_9 = v_8;
  }
  return vec4(v_9, v_6);
}
mat3x2 v_10(uint start_byte_offset) {
  uvec4 v_11 = v_2.inner[(start_byte_offset / 16u)];
  vec2 v_12 = uintBitsToFloat(mix(v_11.xy, v_11.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_13 = v_2.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_14 = uintBitsToFloat(mix(v_13.xy, v_13.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_15 = v_2.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_12, v_14, uintBitsToFloat(mix(v_15.xy, v_15.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat3 v_16(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_GammaTransferParams v_17(uint start_byte_offset) {
  uvec4 v_18 = v_2.inner[(start_byte_offset / 16u)];
  uvec4 v_19 = v_2.inner[((4u + start_byte_offset) / 16u)];
  uvec4 v_20 = v_2.inner[((8u + start_byte_offset) / 16u)];
  uvec4 v_21 = v_2.inner[((12u + start_byte_offset) / 16u)];
  uvec4 v_22 = v_2.inner[((16u + start_byte_offset) / 16u)];
  uvec4 v_23 = v_2.inner[((20u + start_byte_offset) / 16u)];
  uvec4 v_24 = v_2.inner[((24u + start_byte_offset) / 16u)];
  uvec4 v_25 = v_2.inner[((28u + start_byte_offset) / 16u)];
  return tint_GammaTransferParams(uintBitsToFloat(v_18[((start_byte_offset % 16u) / 4u)]), uintBitsToFloat(v_19[(((4u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_20[(((8u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_21[(((12u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_22[(((16u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_23[(((20u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_24[(((24u + start_byte_offset) % 16u) / 4u)]), v_25[(((28u + start_byte_offset) % 16u) / 4u)]);
}
mat3x4 v_26(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_27(uint start_byte_offset) {
  uvec4 v_28 = v_2.inner[(start_byte_offset / 16u)];
  uvec4 v_29 = v_2.inner[((4u + start_byte_offset) / 16u)];
  mat3x4 v_30 = v_26((16u + start_byte_offset));
  tint_GammaTransferParams v_31 = v_17((64u + start_byte_offset));
  tint_GammaTransferParams v_32 = v_17((96u + start_byte_offset));
  mat3 v_33 = v_16((128u + start_byte_offset));
  mat3x2 v_34 = v_10((176u + start_byte_offset));
  mat3x2 v_35 = v_10((200u + start_byte_offset));
  uvec4 v_36 = v_2.inner[((224u + start_byte_offset) / 16u)];
  vec2 v_37 = uintBitsToFloat(mix(v_36.xy, v_36.zw, bvec2(((((224u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_38 = v_2.inner[((232u + start_byte_offset) / 16u)];
  vec2 v_39 = uintBitsToFloat(mix(v_38.xy, v_38.zw, bvec2(((((232u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_40 = v_2.inner[((240u + start_byte_offset) / 16u)];
  vec2 v_41 = uintBitsToFloat(mix(v_40.xy, v_40.zw, bvec2(((((240u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_42 = v_2.inner[((248u + start_byte_offset) / 16u)];
  vec2 v_43 = uintBitsToFloat(mix(v_42.xy, v_42.zw, bvec2(((((248u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_44 = v_2.inner[((256u + start_byte_offset) / 16u)];
  uvec2 v_45 = mix(v_44.xy, v_44.zw, bvec2(((((256u + start_byte_offset) % 16u) / 4u) == 2u)));
  uvec4 v_46 = v_2.inner[((264u + start_byte_offset) / 16u)];
  return tint_ExternalTextureParams(v_28[((start_byte_offset % 16u) / 4u)], v_29[(((4u + start_byte_offset) % 16u) / 4u)], v_30, v_31, v_32, v_33, v_34, v_35, v_37, v_39, v_41, v_43, v_45, uintBitsToFloat(mix(v_46.xy, v_46.zw, bvec2(((((264u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec4 res = tint_TextureSampleExternal(v_27(0u), vec2(1.0f));
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
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_2 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_2) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureSampleExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_3 = (params.sampleTransform * vec3(coords, 1.0f));
  vec3 v_4 = vec3(0.0f);
  float v_5 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_6 = textureLod(v_arg_0_plane0_arg_1, clamp(v_3, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f);
    v_4 = v_6.xyz;
    v_5 = v_6.w;
  } else {
    v_4 = (vec4(textureLod(v_arg_0_plane0_arg_1, clamp(v_3, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f).x, textureLod(v_arg_0_plane1_arg_1, clamp(v_3, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_5 = 1.0f;
  }
  vec3 v_7 = v_4;
  vec3 v_8 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_8 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_7, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_8 = v_7;
  }
  return vec4(v_8, v_5);
}
mat3x2 v_9(uint start_byte_offset) {
  uvec4 v_10 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_11 = uintBitsToFloat(mix(v_10.xy, v_10.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_12 = v_1.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_13 = uintBitsToFloat(mix(v_12.xy, v_12.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_14 = v_1.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_11, v_13, uintBitsToFloat(mix(v_14.xy, v_14.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat3 v_15(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_GammaTransferParams v_16(uint start_byte_offset) {
  uvec4 v_17 = v_1.inner[(start_byte_offset / 16u)];
  uvec4 v_18 = v_1.inner[((4u + start_byte_offset) / 16u)];
  uvec4 v_19 = v_1.inner[((8u + start_byte_offset) / 16u)];
  uvec4 v_20 = v_1.inner[((12u + start_byte_offset) / 16u)];
  uvec4 v_21 = v_1.inner[((16u + start_byte_offset) / 16u)];
  uvec4 v_22 = v_1.inner[((20u + start_byte_offset) / 16u)];
  uvec4 v_23 = v_1.inner[((24u + start_byte_offset) / 16u)];
  uvec4 v_24 = v_1.inner[((28u + start_byte_offset) / 16u)];
  return tint_GammaTransferParams(uintBitsToFloat(v_17[((start_byte_offset % 16u) / 4u)]), uintBitsToFloat(v_18[(((4u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_19[(((8u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_20[(((12u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_21[(((16u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_22[(((20u + start_byte_offset) % 16u) / 4u)]), uintBitsToFloat(v_23[(((24u + start_byte_offset) % 16u) / 4u)]), v_24[(((28u + start_byte_offset) % 16u) / 4u)]);
}
mat3x4 v_25(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_26(uint start_byte_offset) {
  uvec4 v_27 = v_1.inner[(start_byte_offset / 16u)];
  uvec4 v_28 = v_1.inner[((4u + start_byte_offset) / 16u)];
  mat3x4 v_29 = v_25((16u + start_byte_offset));
  tint_GammaTransferParams v_30 = v_16((64u + start_byte_offset));
  tint_GammaTransferParams v_31 = v_16((96u + start_byte_offset));
  mat3 v_32 = v_15((128u + start_byte_offset));
  mat3x2 v_33 = v_9((176u + start_byte_offset));
  mat3x2 v_34 = v_9((200u + start_byte_offset));
  uvec4 v_35 = v_1.inner[((224u + start_byte_offset) / 16u)];
  vec2 v_36 = uintBitsToFloat(mix(v_35.xy, v_35.zw, bvec2(((((224u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_37 = v_1.inner[((232u + start_byte_offset) / 16u)];
  vec2 v_38 = uintBitsToFloat(mix(v_37.xy, v_37.zw, bvec2(((((232u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_39 = v_1.inner[((240u + start_byte_offset) / 16u)];
  vec2 v_40 = uintBitsToFloat(mix(v_39.xy, v_39.zw, bvec2(((((240u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_41 = v_1.inner[((248u + start_byte_offset) / 16u)];
  vec2 v_42 = uintBitsToFloat(mix(v_41.xy, v_41.zw, bvec2(((((248u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_43 = v_1.inner[((256u + start_byte_offset) / 16u)];
  uvec2 v_44 = mix(v_43.xy, v_43.zw, bvec2(((((256u + start_byte_offset) % 16u) / 4u) == 2u)));
  uvec4 v_45 = v_1.inner[((264u + start_byte_offset) / 16u)];
  return tint_ExternalTextureParams(v_27[((start_byte_offset % 16u) / 4u)], v_28[(((4u + start_byte_offset) % 16u) / 4u)], v_29, v_30, v_31, v_32, v_33, v_34, v_36, v_38, v_40, v_42, v_44, uintBitsToFloat(mix(v_45.xy, v_45.zw, bvec2(((((264u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec4 res = tint_TextureSampleExternal(v_26(0u), vec2(1.0f));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_46 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_46.pos = vec4(0.0f);
  v_46.prevent_dce = textureSampleBaseClampToEdge_7c04e6();
  return v_46;
}
void main() {
  VertexOutput v_47 = vertex_main_inner();
  gl_Position = vec4(v_47.pos.x, -(v_47.pos.y), ((2.0f * v_47.pos.z) - v_47.pos.w), v_47.pos.w);
  tint_interstage_location0 = v_47.prevent_dce;
  gl_PointSize = 1.0f;
}
