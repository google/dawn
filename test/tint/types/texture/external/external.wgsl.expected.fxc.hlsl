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
};


cbuffer cbuffer_t_f_params : register(b2) {
  uint4 t_f_params[17];
};
Texture2D<float4> t_f_plane0 : register(t0);
Texture2D<float4> t_f_plane1 : register(t1);
RWByteAddressBuffer v_1 : register(u0, space1);
uint2 tint_v2f32_to_v2u32(float2 value) {
  return uint2(clamp(value, (0.0f).xx, (4294967040.0f).xx));
}

float3 tint_ApplyGammaTransferFunction(float3 v, tint_TransferFunctionParams params) {
  float3 v_2 = float3((params.G).xxx);
  float3 v_3 = float3((params.D).xxx);
  float3 v_4 = abs(v);
  float3 v_5 = float3(sign(v));
  return (((v_4 < v_3)) ? ((v_5 * ((params.C * v_4) + params.F))) : ((v_5 * (pow(((params.A * v_4) + params.B), v_2) + params.E))));
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
  float v_6 = tint_ApplyHLGSingleChannel(v.x, params);
  float v_7 = tint_ApplyHLGSingleChannel(v.y, params);
  return float3(v_6, v_7, tint_ApplyHLGSingleChannel(v.z, params));
}

float3 tint_ApplyPQTransferFunction(float3 v, tint_TransferFunctionParams params) {
  float3 v_8 = float3((params.C).xxx);
  float3 v_9 = float3((params.D).xxx);
  float3 v_10 = float3((params.E).xxx);
  float3 v_11 = float3((params.A).xxx);
  float3 v_12 = pow(clamp(v, (0.0f).xxx, (1.0f).xxx), ((1.0f).xxx / float3((params.B).xxx)));
  return pow((max((v_12 - v_8), (0.0f).xxx) / (v_9 - (v_10 * v_12))), ((1.0f).xxx / v_11));
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
  float2 v_13 = round(mul(float3(float2(min(coords, params.apparentSize)), 1.0f), params.loadTransform));
  uint2 v_14 = tint_v2f32_to_v2u32(v_13);
  float3 v_15 = (0.0f).xxx;
  float v_16 = 0.0f;
  if ((params.numPlanes == 1u)) {
    int2 v_17 = int2(v_14);
    float4 v_18 = plane_0.Load(int3(v_17, int(0u)));
    v_15 = v_18.xyz;
    v_16 = v_18.w;
  } else {
    int2 v_19 = int2(v_14);
    float v_20 = plane_0.Load(int3(v_19, int(0u))).x;
    int2 v_21 = int2(tint_v2f32_to_v2u32((v_13 * params.plane1CoordFactor)));
    v_15 = mul(params.yuvToRgbConversionMatrix, float4(v_20, plane_1.Load(int3(v_21, int(0u))).xy, 1.0f));
    v_16 = 1.0f;
  }
  float3 v_22 = v_15;
  float3 v_23 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_TransferFunctionParams v_24 = params.srcTransferFunction;
    tint_TransferFunctionParams v_25 = params.dstTransferFunction;
    v_23 = tint_ApplyGammaTransferFunction(mul(tint_ApplySrcTransferFunction(v_22, v_24), params.gamutConversionMatrix), v_25);
  } else {
    v_23 = v_22;
  }
  return float4(v_23, v_16);
}

float3x2 v_26(uint start_byte_offset) {
  uint4 v_27 = t_f_params[(start_byte_offset / 16u)];
  float2 v_28 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_27.zw) : (v_27.xy)));
  uint v_29 = (8u + start_byte_offset);
  uint4 v_30 = t_f_params[(v_29 / 16u)];
  float2 v_31 = asfloat((((((v_29 & 15u) >> 2u) == 2u)) ? (v_30.zw) : (v_30.xy)));
  uint v_32 = (16u + start_byte_offset);
  uint4 v_33 = t_f_params[(v_32 / 16u)];
  return float3x2(v_28, v_31, asfloat((((((v_32 & 15u) >> 2u) == 2u)) ? (v_33.zw) : (v_33.xy))));
}

float3x3 v_34(uint start_byte_offset) {
  return float3x3(asfloat(t_f_params[(start_byte_offset / 16u)].xyz), asfloat(t_f_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(t_f_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_35(uint start_byte_offset) {
  uint v_36 = (4u + start_byte_offset);
  uint v_37 = (8u + start_byte_offset);
  uint v_38 = (12u + start_byte_offset);
  uint v_39 = (16u + start_byte_offset);
  uint v_40 = (20u + start_byte_offset);
  uint v_41 = (24u + start_byte_offset);
  uint v_42 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_43 = {t_f_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(t_f_params[(v_36 / 16u)][((v_36 & 15u) >> 2u)]), asfloat(t_f_params[(v_37 / 16u)][((v_37 & 15u) >> 2u)]), asfloat(t_f_params[(v_38 / 16u)][((v_38 & 15u) >> 2u)]), asfloat(t_f_params[(v_39 / 16u)][((v_39 & 15u) >> 2u)]), asfloat(t_f_params[(v_40 / 16u)][((v_40 & 15u) >> 2u)]), asfloat(t_f_params[(v_41 / 16u)][((v_41 & 15u) >> 2u)]), asfloat(t_f_params[(v_42 / 16u)][((v_42 & 15u) >> 2u)])};
  return v_43;
}

float3x4 v_44(uint start_byte_offset) {
  return float3x4(asfloat(t_f_params[(start_byte_offset / 16u)]), asfloat(t_f_params[((16u + start_byte_offset) / 16u)]), asfloat(t_f_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_45(uint start_byte_offset) {
  uint v_46 = t_f_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_47 = (4u + start_byte_offset);
  uint v_48 = t_f_params[(v_47 / 16u)][((v_47 & 15u) >> 2u)];
  float3x4 v_49 = v_44((16u + start_byte_offset));
  tint_TransferFunctionParams v_50 = v_35((64u + start_byte_offset));
  tint_TransferFunctionParams v_51 = v_35((96u + start_byte_offset));
  float3x3 v_52 = v_34((128u + start_byte_offset));
  float3x2 v_53 = v_26((176u + start_byte_offset));
  float3x2 v_54 = v_26((200u + start_byte_offset));
  uint v_55 = (224u + start_byte_offset);
  uint4 v_56 = t_f_params[(v_55 / 16u)];
  float2 v_57 = asfloat((((((v_55 & 15u) >> 2u) == 2u)) ? (v_56.zw) : (v_56.xy)));
  uint v_58 = (232u + start_byte_offset);
  uint4 v_59 = t_f_params[(v_58 / 16u)];
  float2 v_60 = asfloat((((((v_58 & 15u) >> 2u) == 2u)) ? (v_59.zw) : (v_59.xy)));
  uint v_61 = (240u + start_byte_offset);
  uint4 v_62 = t_f_params[(v_61 / 16u)];
  float2 v_63 = asfloat((((((v_61 & 15u) >> 2u) == 2u)) ? (v_62.zw) : (v_62.xy)));
  uint v_64 = (248u + start_byte_offset);
  uint4 v_65 = t_f_params[(v_64 / 16u)];
  float2 v_66 = asfloat((((((v_64 & 15u) >> 2u) == 2u)) ? (v_65.zw) : (v_65.xy)));
  uint v_67 = (256u + start_byte_offset);
  uint4 v_68 = t_f_params[(v_67 / 16u)];
  uint2 v_69 = (((((v_67 & 15u) >> 2u) == 2u)) ? (v_68.zw) : (v_68.xy));
  uint v_70 = (264u + start_byte_offset);
  uint4 v_71 = t_f_params[(v_70 / 16u)];
  tint_ExternalTextureParams v_72 = {v_46, v_48, v_49, v_50, v_51, v_52, v_53, v_54, v_57, v_60, v_63, v_66, v_69, asfloat((((((v_70 & 15u) >> 2u) == 2u)) ? (v_71.zw) : (v_71.xy)))};
  return v_72;
}

[numthreads(1, 1, 1)]
void main() {
  tint_ExternalTextureParams v_73 = v_45(0u);
  v_1.Store4(0u, asuint(tint_TextureLoadMultiplanarExternal(t_f_plane0, t_f_plane1, v_73, uint2((int(0)).xx))));
}

