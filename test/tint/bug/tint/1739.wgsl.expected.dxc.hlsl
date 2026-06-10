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
  float3x4 yuvToRgbConversionMatrix;
  tint_TransferFunctionParams srcTransferFunction;
  tint_TransferFunctionParams dstTransferFunction;
  float3x3 gamutConversionMatrix;
  float3x2 sampleTransform;
  float3x2 loadTransform;
  float2 samplePlane0RectMin;
  float2 samplePlane0RectMax;
  float2 samplePlane1RectMin;
  float2 samplePlane1RectMax;
  uint2 apparentSize;
  float2 plane1CoordFactor;
  float4 ootfParam;
};


cbuffer cbuffer_t_params : register(b3) {
  uint4 t_params[18];
};
Texture2D<float4> t_plane0 : register(t0);
Texture2D<float4> t_plane1 : register(t2);
RWTexture2D<float4> outImage : register(u1);
uint2 tint_v2f32_to_v2u32(float2 value) {
  return uint2(clamp(value, (0.0f).xx, (4294967040.0f).xx));
}

float3 tint_ApplyGammaTransferFunction(float3 v, tint_TransferFunctionParams params) {
  float3 v_1 = float3((params.G).xxx);
  float3 v_2 = float3((params.D).xxx);
  float3 v_3 = abs(v);
  float3 v_4 = float3(sign(v));
  return select((v_3 < v_2), (v_4 * ((params.C * v_3) + params.F)), (v_4 * (pow(((params.A * v_3) + params.B), v_1) + params.E)));
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

float3 tint_ApplyHLGTransferFunction(float3 v, tint_TransferFunctionParams params) {
  float v_5 = tint_ApplyHLGSingleChannel(v.x, params);
  float v_6 = tint_ApplyHLGSingleChannel(v.y, params);
  return float3(v_5, v_6, tint_ApplyHLGSingleChannel(v.z, params));
}

float3 tint_ApplyPQTransferFunction(float3 v, tint_TransferFunctionParams params) {
  float3 v_7 = float3((params.C).xxx);
  float3 v_8 = float3((params.D).xxx);
  float3 v_9 = float3((params.E).xxx);
  float3 v_10 = float3((params.A).xxx);
  float3 v_11 = pow(clamp(v, (0.0f).xxx, (1.0f).xxx), ((1.0f).xxx / float3((params.B).xxx)));
  return pow((max((v_11 - v_7), (0.0f).xxx) / (v_8 - (v_9 * v_11))), ((1.0f).xxx / v_10));
}

float3 tint_ApplySrcTransferFunction(float3 v, tint_TransferFunctionParams params) {
  if ((params.mode == 0u)) {
    return tint_ApplyGammaTransferFunction(v, params);
  } else {
    if ((params.mode == 1u)) {
      return tint_ApplyHLGTransferFunction(v, params);
    } else {
      return tint_ApplyPQTransferFunction(v, params);
    }
    /* unreachable */
    return (0.0f).xxx;
  }
  /* unreachable */
  return (0.0f).xxx;
}

float4 tint_TextureLoadMultiplanarExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, uint2 coords) {
  float2 v_12 = round(mul(float3(float2(min(coords, params.apparentSize)), 1.0f), params.loadTransform));
  uint2 v_13 = tint_v2f32_to_v2u32(v_12);
  float3 v_14 = (0.0f).xxx;
  float v_15 = 0.0f;
  if ((params.numPlanes == 1u)) {
    int2 v_16 = int2(v_13);
    float4 v_17 = plane_0.Load(int3(v_16, int(0u)));
    v_14 = v_17.xyz;
    v_15 = v_17.w;
  } else {
    int2 v_18 = int2(v_13);
    float v_19 = plane_0.Load(int3(v_18, int(0u))).x;
    int2 v_20 = int2(tint_v2f32_to_v2u32((v_12 * params.plane1CoordFactor)));
    v_14 = mul(params.yuvToRgbConversionMatrix, float4(v_19, plane_1.Load(int3(v_20, int(0u))).xy, 1.0f));
    v_15 = 1.0f;
  }
  float3 v_21 = v_14;
  float3 v_22 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_TransferFunctionParams v_23 = params.srcTransferFunction;
    float3 v_24 = tint_ApplySrcTransferFunction(v_21, v_23);
    float3 v_25 = (0.0f).xxx;
    if ((params.ootfParam.w != 0.0f)) {
      float v_26 = dot(params.ootfParam.xyz, v_24);
      v_25 = (v_24 * (float(sign(v_26)) * pow(abs(v_26), params.ootfParam.w)));
    } else {
      v_25 = v_24;
    }
    tint_TransferFunctionParams v_27 = params.dstTransferFunction;
    v_22 = tint_ApplyGammaTransferFunction(mul(v_25, params.gamutConversionMatrix), v_27);
  } else {
    v_22 = v_21;
  }
  return float4(v_22, v_15);
}

float3x2 v_28(uint start_byte_offset) {
  uint4 v_29 = t_params[(start_byte_offset / 16u)];
  uint v_30 = (8u + start_byte_offset);
  uint4 v_31 = t_params[(v_30 / 16u)];
  uint v_32 = (16u + start_byte_offset);
  uint4 v_33 = t_params[(v_32 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_29.zw, v_29.xy)), asfloat(select((((v_30 & 15u) >> 2u) == 2u), v_31.zw, v_31.xy)), asfloat(select((((v_32 & 15u) >> 2u) == 2u), v_33.zw, v_33.xy)));
}

float3x3 v_34(uint start_byte_offset) {
  return float3x3(asfloat(t_params[(start_byte_offset / 16u)].xyz), asfloat(t_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(t_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_35(uint start_byte_offset) {
  uint v_36 = (4u + start_byte_offset);
  uint v_37 = (8u + start_byte_offset);
  uint v_38 = (12u + start_byte_offset);
  uint v_39 = (16u + start_byte_offset);
  uint v_40 = (20u + start_byte_offset);
  uint v_41 = (24u + start_byte_offset);
  uint v_42 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_43 = {t_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(t_params[(v_36 / 16u)][((v_36 & 15u) >> 2u)]), asfloat(t_params[(v_37 / 16u)][((v_37 & 15u) >> 2u)]), asfloat(t_params[(v_38 / 16u)][((v_38 & 15u) >> 2u)]), asfloat(t_params[(v_39 / 16u)][((v_39 & 15u) >> 2u)]), asfloat(t_params[(v_40 / 16u)][((v_40 & 15u) >> 2u)]), asfloat(t_params[(v_41 / 16u)][((v_41 & 15u) >> 2u)]), asfloat(t_params[(v_42 / 16u)][((v_42 & 15u) >> 2u)])};
  return v_43;
}

float3x4 v_44(uint start_byte_offset) {
  return float3x4(asfloat(t_params[(start_byte_offset / 16u)]), asfloat(t_params[((16u + start_byte_offset) / 16u)]), asfloat(t_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_45(uint start_byte_offset) {
  uint v_46 = t_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_47 = (4u + start_byte_offset);
  uint v_48 = t_params[(v_47 / 16u)][((v_47 & 15u) >> 2u)];
  float3x4 v_49 = v_44((16u + start_byte_offset));
  tint_TransferFunctionParams v_50 = v_35((64u + start_byte_offset));
  tint_TransferFunctionParams v_51 = v_35((96u + start_byte_offset));
  float3x3 v_52 = v_34((128u + start_byte_offset));
  float3x2 v_53 = v_28((176u + start_byte_offset));
  float3x2 v_54 = v_28((200u + start_byte_offset));
  uint v_55 = (224u + start_byte_offset);
  uint4 v_56 = t_params[(v_55 / 16u)];
  uint v_57 = (232u + start_byte_offset);
  uint4 v_58 = t_params[(v_57 / 16u)];
  uint v_59 = (240u + start_byte_offset);
  uint4 v_60 = t_params[(v_59 / 16u)];
  uint v_61 = (248u + start_byte_offset);
  uint4 v_62 = t_params[(v_61 / 16u)];
  uint v_63 = (256u + start_byte_offset);
  uint4 v_64 = t_params[(v_63 / 16u)];
  uint v_65 = (264u + start_byte_offset);
  uint4 v_66 = t_params[(v_65 / 16u)];
  tint_ExternalTextureParams v_67 = {v_46, v_48, v_49, v_50, v_51, v_52, v_53, v_54, asfloat(select((((v_55 & 15u) >> 2u) == 2u), v_56.zw, v_56.xy)), asfloat(select((((v_57 & 15u) >> 2u) == 2u), v_58.zw, v_58.xy)), asfloat(select((((v_59 & 15u) >> 2u) == 2u), v_60.zw, v_60.xy)), asfloat(select((((v_61 & 15u) >> 2u) == 2u), v_62.zw, v_62.xy)), select((((v_63 & 15u) >> 2u) == 2u), v_64.zw, v_64.xy), asfloat(select((((v_65 & 15u) >> 2u) == 2u), v_66.zw, v_66.xy)), asfloat(t_params[((272u + start_byte_offset) / 16u)])};
  return v_67;
}

[numthreads(1, 1, 1)]
void main() {
  tint_ExternalTextureParams v_68 = v_45(0u);
  float4 red = tint_TextureLoadMultiplanarExternal(t_plane0, t_plane1, v_68, uint2((int(10)).xx));
  outImage[(int(0)).xx] = red;
  tint_ExternalTextureParams v_69 = v_45(0u);
  float4 green = tint_TextureLoadMultiplanarExternal(t_plane0, t_plane1, v_69, uint2(int2(int(70), int(118))));
  outImage[int2(int(1), int(0))] = green;
}

