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
  uvec2 inner;
} v;
layout(binding = 3, std140)
uniform f_arg_0_params_block_ubo {
  uvec4 inner[18];
} v_1;
uniform highp sampler2D f_arg_0_plane0;
uniform highp sampler2D f_arg_0_plane1;
mat3x2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v_1.inner[(v_5 / 16u)];
  vec2 v_7 = uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u))));
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v_1.inner[(v_8 / 16u)];
  return mat3x2(v_4, v_7, uintBitsToFloat(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u)))));
}
mat3 v_10(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_TransferFunctionParams v_11(uint start_byte_offset) {
  uvec4 v_12 = v_1.inner[(start_byte_offset / 16u)];
  uint v_13 = (4u + start_byte_offset);
  uvec4 v_14 = v_1.inner[(v_13 / 16u)];
  uint v_15 = (8u + start_byte_offset);
  uvec4 v_16 = v_1.inner[(v_15 / 16u)];
  uint v_17 = (12u + start_byte_offset);
  uvec4 v_18 = v_1.inner[(v_17 / 16u)];
  uint v_19 = (16u + start_byte_offset);
  uvec4 v_20 = v_1.inner[(v_19 / 16u)];
  uint v_21 = (20u + start_byte_offset);
  uvec4 v_22 = v_1.inner[(v_21 / 16u)];
  uint v_23 = (24u + start_byte_offset);
  uvec4 v_24 = v_1.inner[(v_23 / 16u)];
  uint v_25 = (28u + start_byte_offset);
  uvec4 v_26 = v_1.inner[(v_25 / 16u)];
  return tint_TransferFunctionParams(v_12[((start_byte_offset & 15u) >> 2u)], uintBitsToFloat(v_14[((v_13 & 15u) >> 2u)]), uintBitsToFloat(v_16[((v_15 & 15u) >> 2u)]), uintBitsToFloat(v_18[((v_17 & 15u) >> 2u)]), uintBitsToFloat(v_20[((v_19 & 15u) >> 2u)]), uintBitsToFloat(v_22[((v_21 & 15u) >> 2u)]), uintBitsToFloat(v_24[((v_23 & 15u) >> 2u)]), uintBitsToFloat(v_26[((v_25 & 15u) >> 2u)]));
}
mat3x4 v_27(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_28(uint start_byte_offset) {
  uvec4 v_29 = v_1.inner[(start_byte_offset / 16u)];
  uint v_30 = (4u + start_byte_offset);
  uvec4 v_31 = v_1.inner[(v_30 / 16u)];
  mat3x4 v_32 = v_27((16u + start_byte_offset));
  tint_TransferFunctionParams v_33 = v_11((64u + start_byte_offset));
  tint_TransferFunctionParams v_34 = v_11((96u + start_byte_offset));
  mat3 v_35 = v_10((128u + start_byte_offset));
  mat3x2 v_36 = v_2((176u + start_byte_offset));
  mat3x2 v_37 = v_2((200u + start_byte_offset));
  uint v_38 = (224u + start_byte_offset);
  uvec4 v_39 = v_1.inner[(v_38 / 16u)];
  vec2 v_40 = uintBitsToFloat(mix(v_39.xy, v_39.zw, bvec2((((v_38 & 15u) >> 2u) == 2u))));
  uint v_41 = (232u + start_byte_offset);
  uvec4 v_42 = v_1.inner[(v_41 / 16u)];
  vec2 v_43 = uintBitsToFloat(mix(v_42.xy, v_42.zw, bvec2((((v_41 & 15u) >> 2u) == 2u))));
  uint v_44 = (240u + start_byte_offset);
  uvec4 v_45 = v_1.inner[(v_44 / 16u)];
  vec2 v_46 = uintBitsToFloat(mix(v_45.xy, v_45.zw, bvec2((((v_44 & 15u) >> 2u) == 2u))));
  uint v_47 = (248u + start_byte_offset);
  uvec4 v_48 = v_1.inner[(v_47 / 16u)];
  vec2 v_49 = uintBitsToFloat(mix(v_48.xy, v_48.zw, bvec2((((v_47 & 15u) >> 2u) == 2u))));
  uint v_50 = (256u + start_byte_offset);
  uvec4 v_51 = v_1.inner[(v_50 / 16u)];
  uvec2 v_52 = mix(v_51.xy, v_51.zw, bvec2((((v_50 & 15u) >> 2u) == 2u)));
  uint v_53 = (264u + start_byte_offset);
  uvec4 v_54 = v_1.inner[(v_53 / 16u)];
  vec2 v_55 = uintBitsToFloat(mix(v_54.xy, v_54.zw, bvec2((((v_53 & 15u) >> 2u) == 2u))));
  return tint_ExternalTextureParams(v_29[((start_byte_offset & 15u) >> 2u)], v_31[((v_30 & 15u) >> 2u)], v_32, v_33, v_34, v_35, v_36, v_37, v_40, v_43, v_46, v_49, v_52, v_55, uintBitsToFloat(v_1.inner[((272u + start_byte_offset) / 16u)]));
}
uvec2 textureDimensions_cdc6c9() {
  uvec2 res = (v_28(0u).apparentSize + uvec2(1u));
  return res;
}
void main() {
  v.inner = textureDimensions_cdc6c9();
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
  uvec2 inner;
} v;
layout(binding = 3, std140)
uniform arg_0_params_block_1_ubo {
  uvec4 inner[18];
} v_1;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
mat3x2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v_1.inner[(v_5 / 16u)];
  vec2 v_7 = uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u))));
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v_1.inner[(v_8 / 16u)];
  return mat3x2(v_4, v_7, uintBitsToFloat(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u)))));
}
mat3 v_10(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_TransferFunctionParams v_11(uint start_byte_offset) {
  uvec4 v_12 = v_1.inner[(start_byte_offset / 16u)];
  uint v_13 = (4u + start_byte_offset);
  uvec4 v_14 = v_1.inner[(v_13 / 16u)];
  uint v_15 = (8u + start_byte_offset);
  uvec4 v_16 = v_1.inner[(v_15 / 16u)];
  uint v_17 = (12u + start_byte_offset);
  uvec4 v_18 = v_1.inner[(v_17 / 16u)];
  uint v_19 = (16u + start_byte_offset);
  uvec4 v_20 = v_1.inner[(v_19 / 16u)];
  uint v_21 = (20u + start_byte_offset);
  uvec4 v_22 = v_1.inner[(v_21 / 16u)];
  uint v_23 = (24u + start_byte_offset);
  uvec4 v_24 = v_1.inner[(v_23 / 16u)];
  uint v_25 = (28u + start_byte_offset);
  uvec4 v_26 = v_1.inner[(v_25 / 16u)];
  return tint_TransferFunctionParams(v_12[((start_byte_offset & 15u) >> 2u)], uintBitsToFloat(v_14[((v_13 & 15u) >> 2u)]), uintBitsToFloat(v_16[((v_15 & 15u) >> 2u)]), uintBitsToFloat(v_18[((v_17 & 15u) >> 2u)]), uintBitsToFloat(v_20[((v_19 & 15u) >> 2u)]), uintBitsToFloat(v_22[((v_21 & 15u) >> 2u)]), uintBitsToFloat(v_24[((v_23 & 15u) >> 2u)]), uintBitsToFloat(v_26[((v_25 & 15u) >> 2u)]));
}
mat3x4 v_27(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_28(uint start_byte_offset) {
  uvec4 v_29 = v_1.inner[(start_byte_offset / 16u)];
  uint v_30 = (4u + start_byte_offset);
  uvec4 v_31 = v_1.inner[(v_30 / 16u)];
  mat3x4 v_32 = v_27((16u + start_byte_offset));
  tint_TransferFunctionParams v_33 = v_11((64u + start_byte_offset));
  tint_TransferFunctionParams v_34 = v_11((96u + start_byte_offset));
  mat3 v_35 = v_10((128u + start_byte_offset));
  mat3x2 v_36 = v_2((176u + start_byte_offset));
  mat3x2 v_37 = v_2((200u + start_byte_offset));
  uint v_38 = (224u + start_byte_offset);
  uvec4 v_39 = v_1.inner[(v_38 / 16u)];
  vec2 v_40 = uintBitsToFloat(mix(v_39.xy, v_39.zw, bvec2((((v_38 & 15u) >> 2u) == 2u))));
  uint v_41 = (232u + start_byte_offset);
  uvec4 v_42 = v_1.inner[(v_41 / 16u)];
  vec2 v_43 = uintBitsToFloat(mix(v_42.xy, v_42.zw, bvec2((((v_41 & 15u) >> 2u) == 2u))));
  uint v_44 = (240u + start_byte_offset);
  uvec4 v_45 = v_1.inner[(v_44 / 16u)];
  vec2 v_46 = uintBitsToFloat(mix(v_45.xy, v_45.zw, bvec2((((v_44 & 15u) >> 2u) == 2u))));
  uint v_47 = (248u + start_byte_offset);
  uvec4 v_48 = v_1.inner[(v_47 / 16u)];
  vec2 v_49 = uintBitsToFloat(mix(v_48.xy, v_48.zw, bvec2((((v_47 & 15u) >> 2u) == 2u))));
  uint v_50 = (256u + start_byte_offset);
  uvec4 v_51 = v_1.inner[(v_50 / 16u)];
  uvec2 v_52 = mix(v_51.xy, v_51.zw, bvec2((((v_50 & 15u) >> 2u) == 2u)));
  uint v_53 = (264u + start_byte_offset);
  uvec4 v_54 = v_1.inner[(v_53 / 16u)];
  vec2 v_55 = uintBitsToFloat(mix(v_54.xy, v_54.zw, bvec2((((v_53 & 15u) >> 2u) == 2u))));
  return tint_ExternalTextureParams(v_29[((start_byte_offset & 15u) >> 2u)], v_31[((v_30 & 15u) >> 2u)], v_32, v_33, v_34, v_35, v_36, v_37, v_40, v_43, v_46, v_49, v_52, v_55, uintBitsToFloat(v_1.inner[((272u + start_byte_offset) / 16u)]));
}
uvec2 textureDimensions_cdc6c9() {
  uvec2 res = (v_28(0u).apparentSize + uvec2(1u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_cdc6c9();
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
  uvec2 prevent_dce;
};

layout(binding = 2, std140)
uniform v_arg_0_params_block_ubo {
  uvec4 inner[18];
} v;
uniform highp sampler2D v_arg_0_plane0;
uniform highp sampler2D v_arg_0_plane1;
layout(location = 0) flat out uvec2 tint_interstage_location0;
mat3x2 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  vec2 v_3 = uintBitsToFloat(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_4 = (8u + start_byte_offset);
  uvec4 v_5 = v.inner[(v_4 / 16u)];
  vec2 v_6 = uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2((((v_4 & 15u) >> 2u) == 2u))));
  uint v_7 = (16u + start_byte_offset);
  uvec4 v_8 = v.inner[(v_7 / 16u)];
  return mat3x2(v_3, v_6, uintBitsToFloat(mix(v_8.xy, v_8.zw, bvec2((((v_7 & 15u) >> 2u) == 2u)))));
}
mat3 v_9(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_TransferFunctionParams v_10(uint start_byte_offset) {
  uvec4 v_11 = v.inner[(start_byte_offset / 16u)];
  uint v_12 = (4u + start_byte_offset);
  uvec4 v_13 = v.inner[(v_12 / 16u)];
  uint v_14 = (8u + start_byte_offset);
  uvec4 v_15 = v.inner[(v_14 / 16u)];
  uint v_16 = (12u + start_byte_offset);
  uvec4 v_17 = v.inner[(v_16 / 16u)];
  uint v_18 = (16u + start_byte_offset);
  uvec4 v_19 = v.inner[(v_18 / 16u)];
  uint v_20 = (20u + start_byte_offset);
  uvec4 v_21 = v.inner[(v_20 / 16u)];
  uint v_22 = (24u + start_byte_offset);
  uvec4 v_23 = v.inner[(v_22 / 16u)];
  uint v_24 = (28u + start_byte_offset);
  uvec4 v_25 = v.inner[(v_24 / 16u)];
  return tint_TransferFunctionParams(v_11[((start_byte_offset & 15u) >> 2u)], uintBitsToFloat(v_13[((v_12 & 15u) >> 2u)]), uintBitsToFloat(v_15[((v_14 & 15u) >> 2u)]), uintBitsToFloat(v_17[((v_16 & 15u) >> 2u)]), uintBitsToFloat(v_19[((v_18 & 15u) >> 2u)]), uintBitsToFloat(v_21[((v_20 & 15u) >> 2u)]), uintBitsToFloat(v_23[((v_22 & 15u) >> 2u)]), uintBitsToFloat(v_25[((v_24 & 15u) >> 2u)]));
}
mat3x4 v_26(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_27(uint start_byte_offset) {
  uvec4 v_28 = v.inner[(start_byte_offset / 16u)];
  uint v_29 = (4u + start_byte_offset);
  uvec4 v_30 = v.inner[(v_29 / 16u)];
  mat3x4 v_31 = v_26((16u + start_byte_offset));
  tint_TransferFunctionParams v_32 = v_10((64u + start_byte_offset));
  tint_TransferFunctionParams v_33 = v_10((96u + start_byte_offset));
  mat3 v_34 = v_9((128u + start_byte_offset));
  mat3x2 v_35 = v_1((176u + start_byte_offset));
  mat3x2 v_36 = v_1((200u + start_byte_offset));
  uint v_37 = (224u + start_byte_offset);
  uvec4 v_38 = v.inner[(v_37 / 16u)];
  vec2 v_39 = uintBitsToFloat(mix(v_38.xy, v_38.zw, bvec2((((v_37 & 15u) >> 2u) == 2u))));
  uint v_40 = (232u + start_byte_offset);
  uvec4 v_41 = v.inner[(v_40 / 16u)];
  vec2 v_42 = uintBitsToFloat(mix(v_41.xy, v_41.zw, bvec2((((v_40 & 15u) >> 2u) == 2u))));
  uint v_43 = (240u + start_byte_offset);
  uvec4 v_44 = v.inner[(v_43 / 16u)];
  vec2 v_45 = uintBitsToFloat(mix(v_44.xy, v_44.zw, bvec2((((v_43 & 15u) >> 2u) == 2u))));
  uint v_46 = (248u + start_byte_offset);
  uvec4 v_47 = v.inner[(v_46 / 16u)];
  vec2 v_48 = uintBitsToFloat(mix(v_47.xy, v_47.zw, bvec2((((v_46 & 15u) >> 2u) == 2u))));
  uint v_49 = (256u + start_byte_offset);
  uvec4 v_50 = v.inner[(v_49 / 16u)];
  uvec2 v_51 = mix(v_50.xy, v_50.zw, bvec2((((v_49 & 15u) >> 2u) == 2u)));
  uint v_52 = (264u + start_byte_offset);
  uvec4 v_53 = v.inner[(v_52 / 16u)];
  vec2 v_54 = uintBitsToFloat(mix(v_53.xy, v_53.zw, bvec2((((v_52 & 15u) >> 2u) == 2u))));
  return tint_ExternalTextureParams(v_28[((start_byte_offset & 15u) >> 2u)], v_30[((v_29 & 15u) >> 2u)], v_31, v_32, v_33, v_34, v_35, v_36, v_39, v_42, v_45, v_48, v_51, v_54, uintBitsToFloat(v.inner[((272u + start_byte_offset) / 16u)]));
}
uvec2 textureDimensions_cdc6c9() {
  uvec2 res = (v_27(0u).apparentSize + uvec2(1u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_55 = VertexOutput(vec4(0.0f), uvec2(0u));
  v_55.pos = vec4(0.0f);
  v_55.prevent_dce = textureDimensions_cdc6c9();
  return v_55;
}
void main() {
  VertexOutput v_56 = vertex_main_inner();
  gl_Position = vec4(v_56.pos.x, -(v_56.pos.y), ((2.0f * v_56.pos.z) - v_56.pos.w), v_56.pos.w);
  tint_interstage_location0 = v_56.prevent_dce;
  gl_PointSize = 1.0f;
}
