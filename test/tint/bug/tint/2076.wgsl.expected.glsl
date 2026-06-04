//
// main
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

layout(binding = 3, std140)
uniform randomTexture_params_block_1_ubo {
  uvec4 inner[17];
} v;
uniform highp sampler2D randomTexture_plane0;
uniform highp sampler2D randomTexture_plane1;
uniform highp sampler2D depthTexture;
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
  return tint_ExternalTextureParams(v_28[((start_byte_offset & 15u) >> 2u)], v_30[((v_29 & 15u) >> 2u)], v_31, v_32, v_33, v_34, v_35, v_36, v_39, v_42, v_45, v_48, v_51, uintBitsToFloat(mix(v_53.xy, v_53.zw, bvec2((((v_52 & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_27(0u);
}
//
// main2
//
#version 310 es

uniform highp sampler2D depthTexture;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
