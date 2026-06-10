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

layout(binding = 3, std140)
uniform t_params_block_1_ubo {
  uvec4 inner[18];
} v_1;
layout(binding = 0, rgba8) uniform highp writeonly image2D outImage;
uniform highp sampler2D t_plane0;
uniform highp sampler2D t_plane1;
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
    vec4 v_17 = texelFetch(t_plane0, v_16, int(0u));
    v_14 = v_17.xyz;
    v_15 = v_17.w;
  } else {
    ivec2 v_18 = ivec2(v_13);
    float v_19 = texelFetch(t_plane0, v_18, int(0u)).x;
    ivec2 v_20 = ivec2(uvec2((v_12 * params.plane1CoordFactor)));
    v_14 = (vec4(v_19, texelFetch(t_plane1, v_20, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
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
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_ExternalTextureParams v_80 = v_52(0u);
  vec4 red = tint_TextureLoadMultiplanarExternal(v_80, min(uvec2(ivec2(10)), ((v_80.apparentSize + uvec2(1u)) - uvec2(1u))));
  imageStore(outImage, ivec2(0), red);
  tint_ExternalTextureParams v_81 = v_52(0u);
  vec4 green = tint_TextureLoadMultiplanarExternal(v_81, min(uvec2(ivec2(70, 118)), ((v_81.apparentSize + uvec2(1u)) - uvec2(1u))));
  imageStore(outImage, ivec2(1, 0), green);
}
