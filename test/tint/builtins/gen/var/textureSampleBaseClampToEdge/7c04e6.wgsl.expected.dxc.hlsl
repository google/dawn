//
// fragment_main
//
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


RWByteAddressBuffer prevent_dce : register(u0);
cbuffer cbuffer_arg_0_params : register(b3, space1) {
  uint4 arg_0_params[18];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t2, space1);
SamplerState arg_1 : register(s1, space1);
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

float4 tint_TextureSampleClampToEdgeMultiplanarExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, SamplerState tint_sampler, float2 coords) {
  float2 v_12 = mul(float3(coords, 1.0f), params.sampleTransform);
  float2 v_13 = clamp(v_12, params.samplePlane0RectMin, params.samplePlane0RectMax);
  float3 v_14 = (0.0f).xxx;
  float v_15 = 0.0f;
  if ((params.numPlanes == 1u)) {
    float4 v_16 = plane_0.SampleLevel(tint_sampler, v_13, 0.0f);
    v_14 = v_16.xyz;
    v_15 = v_16.w;
  } else {
    v_14 = mul(params.yuvToRgbConversionMatrix, float4(plane_0.SampleLevel(tint_sampler, v_13, 0.0f).x, plane_1.SampleLevel(tint_sampler, clamp(v_12, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f));
    v_15 = 1.0f;
  }
  float3 v_17 = v_14;
  float3 v_18 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_TransferFunctionParams v_19 = params.srcTransferFunction;
    float3 v_20 = tint_ApplySrcTransferFunction(v_17, v_19);
    float3 v_21 = (0.0f).xxx;
    if ((params.ootfParam.w != 0.0f)) {
      float v_22 = dot(params.ootfParam.xyz, v_20);
      v_21 = (v_20 * (float(sign(v_22)) * pow(abs(v_22), params.ootfParam.w)));
    } else {
      v_21 = v_20;
    }
    tint_TransferFunctionParams v_23 = params.dstTransferFunction;
    v_18 = tint_ApplyGammaTransferFunction(mul(v_21, params.gamutConversionMatrix), v_23);
  } else {
    v_18 = v_17;
  }
  return float4(v_18, v_15);
}

float3x2 v_24(uint start_byte_offset) {
  uint4 v_25 = arg_0_params[(start_byte_offset / 16u)];
  uint v_26 = (8u + start_byte_offset);
  uint4 v_27 = arg_0_params[(v_26 / 16u)];
  uint v_28 = (16u + start_byte_offset);
  uint4 v_29 = arg_0_params[(v_28 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_25.zw, v_25.xy)), asfloat(select((((v_26 & 15u) >> 2u) == 2u), v_27.zw, v_27.xy)), asfloat(select((((v_28 & 15u) >> 2u) == 2u), v_29.zw, v_29.xy)));
}

float3x3 v_30(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_31(uint start_byte_offset) {
  uint v_32 = (4u + start_byte_offset);
  uint v_33 = (8u + start_byte_offset);
  uint v_34 = (12u + start_byte_offset);
  uint v_35 = (16u + start_byte_offset);
  uint v_36 = (20u + start_byte_offset);
  uint v_37 = (24u + start_byte_offset);
  uint v_38 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_39 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_32 / 16u)][((v_32 & 15u) >> 2u)]), asfloat(arg_0_params[(v_33 / 16u)][((v_33 & 15u) >> 2u)]), asfloat(arg_0_params[(v_34 / 16u)][((v_34 & 15u) >> 2u)]), asfloat(arg_0_params[(v_35 / 16u)][((v_35 & 15u) >> 2u)]), asfloat(arg_0_params[(v_36 / 16u)][((v_36 & 15u) >> 2u)]), asfloat(arg_0_params[(v_37 / 16u)][((v_37 & 15u) >> 2u)]), asfloat(arg_0_params[(v_38 / 16u)][((v_38 & 15u) >> 2u)])};
  return v_39;
}

float3x4 v_40(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_41(uint start_byte_offset) {
  uint v_42 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_43 = (4u + start_byte_offset);
  uint v_44 = arg_0_params[(v_43 / 16u)][((v_43 & 15u) >> 2u)];
  float3x4 v_45 = v_40((16u + start_byte_offset));
  tint_TransferFunctionParams v_46 = v_31((64u + start_byte_offset));
  tint_TransferFunctionParams v_47 = v_31((96u + start_byte_offset));
  float3x3 v_48 = v_30((128u + start_byte_offset));
  float3x2 v_49 = v_24((176u + start_byte_offset));
  float3x2 v_50 = v_24((200u + start_byte_offset));
  uint v_51 = (224u + start_byte_offset);
  uint4 v_52 = arg_0_params[(v_51 / 16u)];
  uint v_53 = (232u + start_byte_offset);
  uint4 v_54 = arg_0_params[(v_53 / 16u)];
  uint v_55 = (240u + start_byte_offset);
  uint4 v_56 = arg_0_params[(v_55 / 16u)];
  uint v_57 = (248u + start_byte_offset);
  uint4 v_58 = arg_0_params[(v_57 / 16u)];
  uint v_59 = (256u + start_byte_offset);
  uint4 v_60 = arg_0_params[(v_59 / 16u)];
  uint v_61 = (264u + start_byte_offset);
  uint4 v_62 = arg_0_params[(v_61 / 16u)];
  tint_ExternalTextureParams v_63 = {v_42, v_44, v_45, v_46, v_47, v_48, v_49, v_50, asfloat(select((((v_51 & 15u) >> 2u) == 2u), v_52.zw, v_52.xy)), asfloat(select((((v_53 & 15u) >> 2u) == 2u), v_54.zw, v_54.xy)), asfloat(select((((v_55 & 15u) >> 2u) == 2u), v_56.zw, v_56.xy)), asfloat(select((((v_57 & 15u) >> 2u) == 2u), v_58.zw, v_58.xy)), select((((v_59 & 15u) >> 2u) == 2u), v_60.zw, v_60.xy), asfloat(select((((v_61 & 15u) >> 2u) == 2u), v_62.zw, v_62.xy)), asfloat(arg_0_params[((272u + start_byte_offset) / 16u)])};
  return v_63;
}

float4 textureSampleBaseClampToEdge_7c04e6() {
  float2 arg_2 = (1.0f).xx;
  tint_ExternalTextureParams v_64 = v_41(0u);
  float4 res = tint_TextureSampleClampToEdgeMultiplanarExternal(arg_0_plane0, arg_0_plane1, v_64, arg_1, arg_2);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBaseClampToEdge_7c04e6()));
}

//
// compute_main
//
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


RWByteAddressBuffer prevent_dce : register(u0);
cbuffer cbuffer_arg_0_params : register(b3, space1) {
  uint4 arg_0_params[18];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t2, space1);
SamplerState arg_1 : register(s1, space1);
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

float4 tint_TextureSampleClampToEdgeMultiplanarExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, SamplerState tint_sampler, float2 coords) {
  float2 v_12 = mul(float3(coords, 1.0f), params.sampleTransform);
  float2 v_13 = clamp(v_12, params.samplePlane0RectMin, params.samplePlane0RectMax);
  float3 v_14 = (0.0f).xxx;
  float v_15 = 0.0f;
  if ((params.numPlanes == 1u)) {
    float4 v_16 = plane_0.SampleLevel(tint_sampler, v_13, 0.0f);
    v_14 = v_16.xyz;
    v_15 = v_16.w;
  } else {
    v_14 = mul(params.yuvToRgbConversionMatrix, float4(plane_0.SampleLevel(tint_sampler, v_13, 0.0f).x, plane_1.SampleLevel(tint_sampler, clamp(v_12, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f));
    v_15 = 1.0f;
  }
  float3 v_17 = v_14;
  float3 v_18 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_TransferFunctionParams v_19 = params.srcTransferFunction;
    float3 v_20 = tint_ApplySrcTransferFunction(v_17, v_19);
    float3 v_21 = (0.0f).xxx;
    if ((params.ootfParam.w != 0.0f)) {
      float v_22 = dot(params.ootfParam.xyz, v_20);
      v_21 = (v_20 * (float(sign(v_22)) * pow(abs(v_22), params.ootfParam.w)));
    } else {
      v_21 = v_20;
    }
    tint_TransferFunctionParams v_23 = params.dstTransferFunction;
    v_18 = tint_ApplyGammaTransferFunction(mul(v_21, params.gamutConversionMatrix), v_23);
  } else {
    v_18 = v_17;
  }
  return float4(v_18, v_15);
}

float3x2 v_24(uint start_byte_offset) {
  uint4 v_25 = arg_0_params[(start_byte_offset / 16u)];
  uint v_26 = (8u + start_byte_offset);
  uint4 v_27 = arg_0_params[(v_26 / 16u)];
  uint v_28 = (16u + start_byte_offset);
  uint4 v_29 = arg_0_params[(v_28 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_25.zw, v_25.xy)), asfloat(select((((v_26 & 15u) >> 2u) == 2u), v_27.zw, v_27.xy)), asfloat(select((((v_28 & 15u) >> 2u) == 2u), v_29.zw, v_29.xy)));
}

float3x3 v_30(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_31(uint start_byte_offset) {
  uint v_32 = (4u + start_byte_offset);
  uint v_33 = (8u + start_byte_offset);
  uint v_34 = (12u + start_byte_offset);
  uint v_35 = (16u + start_byte_offset);
  uint v_36 = (20u + start_byte_offset);
  uint v_37 = (24u + start_byte_offset);
  uint v_38 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_39 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_32 / 16u)][((v_32 & 15u) >> 2u)]), asfloat(arg_0_params[(v_33 / 16u)][((v_33 & 15u) >> 2u)]), asfloat(arg_0_params[(v_34 / 16u)][((v_34 & 15u) >> 2u)]), asfloat(arg_0_params[(v_35 / 16u)][((v_35 & 15u) >> 2u)]), asfloat(arg_0_params[(v_36 / 16u)][((v_36 & 15u) >> 2u)]), asfloat(arg_0_params[(v_37 / 16u)][((v_37 & 15u) >> 2u)]), asfloat(arg_0_params[(v_38 / 16u)][((v_38 & 15u) >> 2u)])};
  return v_39;
}

float3x4 v_40(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_41(uint start_byte_offset) {
  uint v_42 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_43 = (4u + start_byte_offset);
  uint v_44 = arg_0_params[(v_43 / 16u)][((v_43 & 15u) >> 2u)];
  float3x4 v_45 = v_40((16u + start_byte_offset));
  tint_TransferFunctionParams v_46 = v_31((64u + start_byte_offset));
  tint_TransferFunctionParams v_47 = v_31((96u + start_byte_offset));
  float3x3 v_48 = v_30((128u + start_byte_offset));
  float3x2 v_49 = v_24((176u + start_byte_offset));
  float3x2 v_50 = v_24((200u + start_byte_offset));
  uint v_51 = (224u + start_byte_offset);
  uint4 v_52 = arg_0_params[(v_51 / 16u)];
  uint v_53 = (232u + start_byte_offset);
  uint4 v_54 = arg_0_params[(v_53 / 16u)];
  uint v_55 = (240u + start_byte_offset);
  uint4 v_56 = arg_0_params[(v_55 / 16u)];
  uint v_57 = (248u + start_byte_offset);
  uint4 v_58 = arg_0_params[(v_57 / 16u)];
  uint v_59 = (256u + start_byte_offset);
  uint4 v_60 = arg_0_params[(v_59 / 16u)];
  uint v_61 = (264u + start_byte_offset);
  uint4 v_62 = arg_0_params[(v_61 / 16u)];
  tint_ExternalTextureParams v_63 = {v_42, v_44, v_45, v_46, v_47, v_48, v_49, v_50, asfloat(select((((v_51 & 15u) >> 2u) == 2u), v_52.zw, v_52.xy)), asfloat(select((((v_53 & 15u) >> 2u) == 2u), v_54.zw, v_54.xy)), asfloat(select((((v_55 & 15u) >> 2u) == 2u), v_56.zw, v_56.xy)), asfloat(select((((v_57 & 15u) >> 2u) == 2u), v_58.zw, v_58.xy)), select((((v_59 & 15u) >> 2u) == 2u), v_60.zw, v_60.xy), asfloat(select((((v_61 & 15u) >> 2u) == 2u), v_62.zw, v_62.xy)), asfloat(arg_0_params[((272u + start_byte_offset) / 16u)])};
  return v_63;
}

float4 textureSampleBaseClampToEdge_7c04e6() {
  float2 arg_2 = (1.0f).xx;
  tint_ExternalTextureParams v_64 = v_41(0u);
  float4 res = tint_TextureSampleClampToEdgeMultiplanarExternal(arg_0_plane0, arg_0_plane1, v_64, arg_1, arg_2);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBaseClampToEdge_7c04e6()));
}

//
// vertex_main
//
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

struct VertexOutput {
  float4 pos;
  float4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


cbuffer cbuffer_arg_0_params : register(b3, space1) {
  uint4 arg_0_params[18];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t2, space1);
SamplerState arg_1 : register(s1, space1);
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

float4 tint_TextureSampleClampToEdgeMultiplanarExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, SamplerState tint_sampler, float2 coords) {
  float2 v_12 = mul(float3(coords, 1.0f), params.sampleTransform);
  float2 v_13 = clamp(v_12, params.samplePlane0RectMin, params.samplePlane0RectMax);
  float3 v_14 = (0.0f).xxx;
  float v_15 = 0.0f;
  if ((params.numPlanes == 1u)) {
    float4 v_16 = plane_0.SampleLevel(tint_sampler, v_13, 0.0f);
    v_14 = v_16.xyz;
    v_15 = v_16.w;
  } else {
    v_14 = mul(params.yuvToRgbConversionMatrix, float4(plane_0.SampleLevel(tint_sampler, v_13, 0.0f).x, plane_1.SampleLevel(tint_sampler, clamp(v_12, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f));
    v_15 = 1.0f;
  }
  float3 v_17 = v_14;
  float3 v_18 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_TransferFunctionParams v_19 = params.srcTransferFunction;
    float3 v_20 = tint_ApplySrcTransferFunction(v_17, v_19);
    float3 v_21 = (0.0f).xxx;
    if ((params.ootfParam.w != 0.0f)) {
      float v_22 = dot(params.ootfParam.xyz, v_20);
      v_21 = (v_20 * (float(sign(v_22)) * pow(abs(v_22), params.ootfParam.w)));
    } else {
      v_21 = v_20;
    }
    tint_TransferFunctionParams v_23 = params.dstTransferFunction;
    v_18 = tint_ApplyGammaTransferFunction(mul(v_21, params.gamutConversionMatrix), v_23);
  } else {
    v_18 = v_17;
  }
  return float4(v_18, v_15);
}

float3x2 v_24(uint start_byte_offset) {
  uint4 v_25 = arg_0_params[(start_byte_offset / 16u)];
  uint v_26 = (8u + start_byte_offset);
  uint4 v_27 = arg_0_params[(v_26 / 16u)];
  uint v_28 = (16u + start_byte_offset);
  uint4 v_29 = arg_0_params[(v_28 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_25.zw, v_25.xy)), asfloat(select((((v_26 & 15u) >> 2u) == 2u), v_27.zw, v_27.xy)), asfloat(select((((v_28 & 15u) >> 2u) == 2u), v_29.zw, v_29.xy)));
}

float3x3 v_30(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_31(uint start_byte_offset) {
  uint v_32 = (4u + start_byte_offset);
  uint v_33 = (8u + start_byte_offset);
  uint v_34 = (12u + start_byte_offset);
  uint v_35 = (16u + start_byte_offset);
  uint v_36 = (20u + start_byte_offset);
  uint v_37 = (24u + start_byte_offset);
  uint v_38 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_39 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_32 / 16u)][((v_32 & 15u) >> 2u)]), asfloat(arg_0_params[(v_33 / 16u)][((v_33 & 15u) >> 2u)]), asfloat(arg_0_params[(v_34 / 16u)][((v_34 & 15u) >> 2u)]), asfloat(arg_0_params[(v_35 / 16u)][((v_35 & 15u) >> 2u)]), asfloat(arg_0_params[(v_36 / 16u)][((v_36 & 15u) >> 2u)]), asfloat(arg_0_params[(v_37 / 16u)][((v_37 & 15u) >> 2u)]), asfloat(arg_0_params[(v_38 / 16u)][((v_38 & 15u) >> 2u)])};
  return v_39;
}

float3x4 v_40(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_41(uint start_byte_offset) {
  uint v_42 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_43 = (4u + start_byte_offset);
  uint v_44 = arg_0_params[(v_43 / 16u)][((v_43 & 15u) >> 2u)];
  float3x4 v_45 = v_40((16u + start_byte_offset));
  tint_TransferFunctionParams v_46 = v_31((64u + start_byte_offset));
  tint_TransferFunctionParams v_47 = v_31((96u + start_byte_offset));
  float3x3 v_48 = v_30((128u + start_byte_offset));
  float3x2 v_49 = v_24((176u + start_byte_offset));
  float3x2 v_50 = v_24((200u + start_byte_offset));
  uint v_51 = (224u + start_byte_offset);
  uint4 v_52 = arg_0_params[(v_51 / 16u)];
  uint v_53 = (232u + start_byte_offset);
  uint4 v_54 = arg_0_params[(v_53 / 16u)];
  uint v_55 = (240u + start_byte_offset);
  uint4 v_56 = arg_0_params[(v_55 / 16u)];
  uint v_57 = (248u + start_byte_offset);
  uint4 v_58 = arg_0_params[(v_57 / 16u)];
  uint v_59 = (256u + start_byte_offset);
  uint4 v_60 = arg_0_params[(v_59 / 16u)];
  uint v_61 = (264u + start_byte_offset);
  uint4 v_62 = arg_0_params[(v_61 / 16u)];
  tint_ExternalTextureParams v_63 = {v_42, v_44, v_45, v_46, v_47, v_48, v_49, v_50, asfloat(select((((v_51 & 15u) >> 2u) == 2u), v_52.zw, v_52.xy)), asfloat(select((((v_53 & 15u) >> 2u) == 2u), v_54.zw, v_54.xy)), asfloat(select((((v_55 & 15u) >> 2u) == 2u), v_56.zw, v_56.xy)), asfloat(select((((v_57 & 15u) >> 2u) == 2u), v_58.zw, v_58.xy)), select((((v_59 & 15u) >> 2u) == 2u), v_60.zw, v_60.xy), asfloat(select((((v_61 & 15u) >> 2u) == 2u), v_62.zw, v_62.xy)), asfloat(arg_0_params[((272u + start_byte_offset) / 16u)])};
  return v_63;
}

float4 textureSampleBaseClampToEdge_7c04e6() {
  float2 arg_2 = (1.0f).xx;
  tint_ExternalTextureParams v_64 = v_41(0u);
  float4 res = tint_TextureSampleClampToEdgeMultiplanarExternal(arg_0_plane0, arg_0_plane1, v_64, arg_1, arg_2);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_65 = (VertexOutput)0;
  v_65.pos = (0.0f).xxxx;
  v_65.prevent_dce = textureSampleBaseClampToEdge_7c04e6();
  VertexOutput v_66 = v_65;
  return v_66;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_67 = vertex_main_inner();
  vertex_main_outputs v_68 = {v_67.prevent_dce, v_67.pos};
  return v_68;
}

