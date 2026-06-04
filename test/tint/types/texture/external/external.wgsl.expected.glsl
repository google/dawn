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
uniform t_f_params_block_1_ubo {
  uvec4 inner[17];
} v_1;
layout(binding = 0, std430)
buffer out_block_1_ssbo {
  vec4 inner;
} v_2;
uniform highp sampler2D t_f_plane0;
uniform highp sampler2D t_f_plane1;
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
    vec4 v_18 = texelFetch(t_f_plane0, v_17, int(0u));
    v_15 = v_18.xyz;
    v_16 = v_18.w;
  } else {
    ivec2 v_19 = ivec2(v_14);
    float v_20 = texelFetch(t_f_plane0, v_19, int(0u)).x;
    ivec2 v_21 = ivec2(uvec2((v_13 * params.plane1CoordFactor)));
    v_15 = (vec4(v_20, texelFetch(t_f_plane1, v_21, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_16 = 1.0f;
  }
  vec3 v_22 = v_15;
  vec3 v_23 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_23 = tint_ApplyGammaTransferFunction((params.gamutConversionMatrix * tint_ApplySrcTransferFunction(v_22, params.srcTransferFunction)), params.dstTransferFunction);
  } else {
    v_23 = v_22;
  }
  return vec4(v_23, v_16);
}
mat3x2 v_24(uint start_byte_offset) {
  uvec4 v_25 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_26 = uintBitsToFloat(mix(v_25.xy, v_25.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_27 = (8u + start_byte_offset);
  uvec4 v_28 = v_1.inner[(v_27 / 16u)];
  vec2 v_29 = uintBitsToFloat(mix(v_28.xy, v_28.zw, bvec2((((v_27 & 15u) >> 2u) == 2u))));
  uint v_30 = (16u + start_byte_offset);
  uvec4 v_31 = v_1.inner[(v_30 / 16u)];
  return mat3x2(v_26, v_29, uintBitsToFloat(mix(v_31.xy, v_31.zw, bvec2((((v_30 & 15u) >> 2u) == 2u)))));
}
mat3 v_32(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz));
}
tint_TransferFunctionParams v_33(uint start_byte_offset) {
  uvec4 v_34 = v_1.inner[(start_byte_offset / 16u)];
  uint v_35 = (4u + start_byte_offset);
  uvec4 v_36 = v_1.inner[(v_35 / 16u)];
  uint v_37 = (8u + start_byte_offset);
  uvec4 v_38 = v_1.inner[(v_37 / 16u)];
  uint v_39 = (12u + start_byte_offset);
  uvec4 v_40 = v_1.inner[(v_39 / 16u)];
  uint v_41 = (16u + start_byte_offset);
  uvec4 v_42 = v_1.inner[(v_41 / 16u)];
  uint v_43 = (20u + start_byte_offset);
  uvec4 v_44 = v_1.inner[(v_43 / 16u)];
  uint v_45 = (24u + start_byte_offset);
  uvec4 v_46 = v_1.inner[(v_45 / 16u)];
  uint v_47 = (28u + start_byte_offset);
  uvec4 v_48 = v_1.inner[(v_47 / 16u)];
  return tint_TransferFunctionParams(v_34[((start_byte_offset & 15u) >> 2u)], uintBitsToFloat(v_36[((v_35 & 15u) >> 2u)]), uintBitsToFloat(v_38[((v_37 & 15u) >> 2u)]), uintBitsToFloat(v_40[((v_39 & 15u) >> 2u)]), uintBitsToFloat(v_42[((v_41 & 15u) >> 2u)]), uintBitsToFloat(v_44[((v_43 & 15u) >> 2u)]), uintBitsToFloat(v_46[((v_45 & 15u) >> 2u)]), uintBitsToFloat(v_48[((v_47 & 15u) >> 2u)]));
}
mat3x4 v_49(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)]));
}
tint_ExternalTextureParams v_50(uint start_byte_offset) {
  uvec4 v_51 = v_1.inner[(start_byte_offset / 16u)];
  uint v_52 = (4u + start_byte_offset);
  uvec4 v_53 = v_1.inner[(v_52 / 16u)];
  mat3x4 v_54 = v_49((16u + start_byte_offset));
  tint_TransferFunctionParams v_55 = v_33((64u + start_byte_offset));
  tint_TransferFunctionParams v_56 = v_33((96u + start_byte_offset));
  mat3 v_57 = v_32((128u + start_byte_offset));
  mat3x2 v_58 = v_24((176u + start_byte_offset));
  mat3x2 v_59 = v_24((200u + start_byte_offset));
  uint v_60 = (224u + start_byte_offset);
  uvec4 v_61 = v_1.inner[(v_60 / 16u)];
  vec2 v_62 = uintBitsToFloat(mix(v_61.xy, v_61.zw, bvec2((((v_60 & 15u) >> 2u) == 2u))));
  uint v_63 = (232u + start_byte_offset);
  uvec4 v_64 = v_1.inner[(v_63 / 16u)];
  vec2 v_65 = uintBitsToFloat(mix(v_64.xy, v_64.zw, bvec2((((v_63 & 15u) >> 2u) == 2u))));
  uint v_66 = (240u + start_byte_offset);
  uvec4 v_67 = v_1.inner[(v_66 / 16u)];
  vec2 v_68 = uintBitsToFloat(mix(v_67.xy, v_67.zw, bvec2((((v_66 & 15u) >> 2u) == 2u))));
  uint v_69 = (248u + start_byte_offset);
  uvec4 v_70 = v_1.inner[(v_69 / 16u)];
  vec2 v_71 = uintBitsToFloat(mix(v_70.xy, v_70.zw, bvec2((((v_69 & 15u) >> 2u) == 2u))));
  uint v_72 = (256u + start_byte_offset);
  uvec4 v_73 = v_1.inner[(v_72 / 16u)];
  uvec2 v_74 = mix(v_73.xy, v_73.zw, bvec2((((v_72 & 15u) >> 2u) == 2u)));
  uint v_75 = (264u + start_byte_offset);
  uvec4 v_76 = v_1.inner[(v_75 / 16u)];
  return tint_ExternalTextureParams(v_51[((start_byte_offset & 15u) >> 2u)], v_53[((v_52 & 15u) >> 2u)], v_54, v_55, v_56, v_57, v_58, v_59, v_62, v_65, v_68, v_71, v_74, uintBitsToFloat(mix(v_76.xy, v_76.zw, bvec2((((v_75 & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_ExternalTextureParams v_77 = v_50(0u);
  v_2.inner = tint_TextureLoadMultiplanarExternal(v_77, min(uvec2(ivec2(0)), ((v_77.apparentSize + uvec2(1u)) - uvec2(1u))));
}
