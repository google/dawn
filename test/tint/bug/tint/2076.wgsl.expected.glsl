//
// main
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

uniform highp sampler2D randomTexture_plane0;
uniform highp sampler2D randomTexture_plane1;
layout(binding = 3, std140)
uniform randomTexture_params_block_1_ubo {
  uvec4 inner[17];
} v;
uniform highp sampler2D depthTexture;
mat3x2 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  vec2 v_3 = uintBitsToFloat(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_4 = v.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_5 = uintBitsToFloat(mix(v_4.xy, v_4.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_6 = v.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_3, v_5, uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
mat3 v_7(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_GammaTransferParams v_8(uint start_byte_offset) {
  uvec4 v_9 = v.inner[(start_byte_offset / 16u)];
  uvec4 v_10 = v.inner[((4u + start_byte_offset) / 16u)];
  uvec4 v_11 = v.inner[((8u + start_byte_offset) / 16u)];
  uvec4 v_12 = v.inner[((12u + start_byte_offset) / 16u)];
  uvec4 v_13 = v.inner[((16u + start_byte_offset) / 16u)];
  uvec4 v_14 = v.inner[((20u + start_byte_offset) / 16u)];
  uvec4 v_15 = v.inner[((24u + start_byte_offset) / 16u)];
  uvec4 v_16 = v.inner[((28u + start_byte_offset) / 16u)];
  return tint_GammaTransferParams(uintBitsToFloat(v_9[((start_byte_offset & 15u) >> 2u)]), uintBitsToFloat(v_10[(((4u + start_byte_offset) & 15u) >> 2u)]), uintBitsToFloat(v_11[(((8u + start_byte_offset) & 15u) >> 2u)]), uintBitsToFloat(v_12[(((12u + start_byte_offset) & 15u) >> 2u)]), uintBitsToFloat(v_13[(((16u + start_byte_offset) & 15u) >> 2u)]), uintBitsToFloat(v_14[(((20u + start_byte_offset) & 15u) >> 2u)]), uintBitsToFloat(v_15[(((24u + start_byte_offset) & 15u) >> 2u)]), v_16[(((28u + start_byte_offset) & 15u) >> 2u)]);
}
mat3x4 v_17(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_18(uint start_byte_offset) {
  uvec4 v_19 = v.inner[(start_byte_offset / 16u)];
  uvec4 v_20 = v.inner[((4u + start_byte_offset) / 16u)];
  mat3x4 v_21 = v_17((16u + start_byte_offset));
  tint_GammaTransferParams v_22 = v_8((64u + start_byte_offset));
  tint_GammaTransferParams v_23 = v_8((96u + start_byte_offset));
  mat3 v_24 = v_7((128u + start_byte_offset));
  mat3x2 v_25 = v_1((176u + start_byte_offset));
  mat3x2 v_26 = v_1((200u + start_byte_offset));
  uvec4 v_27 = v.inner[((224u + start_byte_offset) / 16u)];
  vec2 v_28 = uintBitsToFloat(mix(v_27.xy, v_27.zw, bvec2(((((224u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_29 = v.inner[((232u + start_byte_offset) / 16u)];
  vec2 v_30 = uintBitsToFloat(mix(v_29.xy, v_29.zw, bvec2(((((232u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_31 = v.inner[((240u + start_byte_offset) / 16u)];
  vec2 v_32 = uintBitsToFloat(mix(v_31.xy, v_31.zw, bvec2(((((240u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_33 = v.inner[((248u + start_byte_offset) / 16u)];
  vec2 v_34 = uintBitsToFloat(mix(v_33.xy, v_33.zw, bvec2(((((248u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_35 = v.inner[((256u + start_byte_offset) / 16u)];
  uvec2 v_36 = mix(v_35.xy, v_35.zw, bvec2(((((256u + start_byte_offset) & 15u) >> 2u) == 2u)));
  uvec4 v_37 = v.inner[((264u + start_byte_offset) / 16u)];
  return tint_ExternalTextureParams(v_19[((start_byte_offset & 15u) >> 2u)], v_20[(((4u + start_byte_offset) & 15u) >> 2u)], v_21, v_22, v_23, v_24, v_25, v_26, v_28, v_30, v_32, v_34, v_36, uintBitsToFloat(mix(v_37.xy, v_37.zw, bvec2(((((264u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_18(0u);
}
//
// main2
//
#version 310 es

uniform highp sampler2D depthTexture;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
