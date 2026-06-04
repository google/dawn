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
};


RWByteAddressBuffer prevent_dce : register(u0);
cbuffer cbuffer_arg_0_params : register(b3, space1) {
  uint4 arg_0_params[17];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t2, space1);
SamplerState arg_1 : register(s1, space1);
float3 tint_ApplyGammaTransferFunction(float3 v, tint_TransferFunctionParams params) {
  float3 v_1 = float3((params.G).xxx);
  float3 v_2 = float3((params.D).xxx);
  float3 v_3 = abs(v);
  float3 v_4 = float3(sign(v));
  return (((v_3 < v_2)) ? ((v_4 * ((params.C * v_3) + params.F))) : ((v_4 * (pow(((params.A * v_3) + params.B), v_1) + params.E))));
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
    tint_TransferFunctionParams v_20 = params.dstTransferFunction;
    v_18 = tint_ApplyGammaTransferFunction(mul(tint_ApplySrcTransferFunction(v_17, v_19), params.gamutConversionMatrix), v_20);
  } else {
    v_18 = v_17;
  }
  return float4(v_18, v_15);
}

float3x2 v_21(uint start_byte_offset) {
  uint4 v_22 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_23 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_22.zw) : (v_22.xy)));
  uint v_24 = (8u + start_byte_offset);
  uint4 v_25 = arg_0_params[(v_24 / 16u)];
  float2 v_26 = asfloat((((((v_24 & 15u) >> 2u) == 2u)) ? (v_25.zw) : (v_25.xy)));
  uint v_27 = (16u + start_byte_offset);
  uint4 v_28 = arg_0_params[(v_27 / 16u)];
  return float3x2(v_23, v_26, asfloat((((((v_27 & 15u) >> 2u) == 2u)) ? (v_28.zw) : (v_28.xy))));
}

float3x3 v_29(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_30(uint start_byte_offset) {
  uint v_31 = (4u + start_byte_offset);
  uint v_32 = (8u + start_byte_offset);
  uint v_33 = (12u + start_byte_offset);
  uint v_34 = (16u + start_byte_offset);
  uint v_35 = (20u + start_byte_offset);
  uint v_36 = (24u + start_byte_offset);
  uint v_37 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_38 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_31 / 16u)][((v_31 & 15u) >> 2u)]), asfloat(arg_0_params[(v_32 / 16u)][((v_32 & 15u) >> 2u)]), asfloat(arg_0_params[(v_33 / 16u)][((v_33 & 15u) >> 2u)]), asfloat(arg_0_params[(v_34 / 16u)][((v_34 & 15u) >> 2u)]), asfloat(arg_0_params[(v_35 / 16u)][((v_35 & 15u) >> 2u)]), asfloat(arg_0_params[(v_36 / 16u)][((v_36 & 15u) >> 2u)]), asfloat(arg_0_params[(v_37 / 16u)][((v_37 & 15u) >> 2u)])};
  return v_38;
}

float3x4 v_39(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_40(uint start_byte_offset) {
  uint v_41 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_42 = (4u + start_byte_offset);
  uint v_43 = arg_0_params[(v_42 / 16u)][((v_42 & 15u) >> 2u)];
  float3x4 v_44 = v_39((16u + start_byte_offset));
  tint_TransferFunctionParams v_45 = v_30((64u + start_byte_offset));
  tint_TransferFunctionParams v_46 = v_30((96u + start_byte_offset));
  float3x3 v_47 = v_29((128u + start_byte_offset));
  float3x2 v_48 = v_21((176u + start_byte_offset));
  float3x2 v_49 = v_21((200u + start_byte_offset));
  uint v_50 = (224u + start_byte_offset);
  uint4 v_51 = arg_0_params[(v_50 / 16u)];
  float2 v_52 = asfloat((((((v_50 & 15u) >> 2u) == 2u)) ? (v_51.zw) : (v_51.xy)));
  uint v_53 = (232u + start_byte_offset);
  uint4 v_54 = arg_0_params[(v_53 / 16u)];
  float2 v_55 = asfloat((((((v_53 & 15u) >> 2u) == 2u)) ? (v_54.zw) : (v_54.xy)));
  uint v_56 = (240u + start_byte_offset);
  uint4 v_57 = arg_0_params[(v_56 / 16u)];
  float2 v_58 = asfloat((((((v_56 & 15u) >> 2u) == 2u)) ? (v_57.zw) : (v_57.xy)));
  uint v_59 = (248u + start_byte_offset);
  uint4 v_60 = arg_0_params[(v_59 / 16u)];
  float2 v_61 = asfloat((((((v_59 & 15u) >> 2u) == 2u)) ? (v_60.zw) : (v_60.xy)));
  uint v_62 = (256u + start_byte_offset);
  uint4 v_63 = arg_0_params[(v_62 / 16u)];
  uint2 v_64 = (((((v_62 & 15u) >> 2u) == 2u)) ? (v_63.zw) : (v_63.xy));
  uint v_65 = (264u + start_byte_offset);
  uint4 v_66 = arg_0_params[(v_65 / 16u)];
  tint_ExternalTextureParams v_67 = {v_41, v_43, v_44, v_45, v_46, v_47, v_48, v_49, v_52, v_55, v_58, v_61, v_64, asfloat((((((v_65 & 15u) >> 2u) == 2u)) ? (v_66.zw) : (v_66.xy)))};
  return v_67;
}

float4 textureSampleBaseClampToEdge_7c04e6() {
  tint_ExternalTextureParams v_68 = v_40(0u);
  float4 res = tint_TextureSampleClampToEdgeMultiplanarExternal(arg_0_plane0, arg_0_plane1, v_68, arg_1, (1.0f).xx);
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
};


RWByteAddressBuffer prevent_dce : register(u0);
cbuffer cbuffer_arg_0_params : register(b3, space1) {
  uint4 arg_0_params[17];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t2, space1);
SamplerState arg_1 : register(s1, space1);
float3 tint_ApplyGammaTransferFunction(float3 v, tint_TransferFunctionParams params) {
  float3 v_1 = float3((params.G).xxx);
  float3 v_2 = float3((params.D).xxx);
  float3 v_3 = abs(v);
  float3 v_4 = float3(sign(v));
  return (((v_3 < v_2)) ? ((v_4 * ((params.C * v_3) + params.F))) : ((v_4 * (pow(((params.A * v_3) + params.B), v_1) + params.E))));
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
    tint_TransferFunctionParams v_20 = params.dstTransferFunction;
    v_18 = tint_ApplyGammaTransferFunction(mul(tint_ApplySrcTransferFunction(v_17, v_19), params.gamutConversionMatrix), v_20);
  } else {
    v_18 = v_17;
  }
  return float4(v_18, v_15);
}

float3x2 v_21(uint start_byte_offset) {
  uint4 v_22 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_23 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_22.zw) : (v_22.xy)));
  uint v_24 = (8u + start_byte_offset);
  uint4 v_25 = arg_0_params[(v_24 / 16u)];
  float2 v_26 = asfloat((((((v_24 & 15u) >> 2u) == 2u)) ? (v_25.zw) : (v_25.xy)));
  uint v_27 = (16u + start_byte_offset);
  uint4 v_28 = arg_0_params[(v_27 / 16u)];
  return float3x2(v_23, v_26, asfloat((((((v_27 & 15u) >> 2u) == 2u)) ? (v_28.zw) : (v_28.xy))));
}

float3x3 v_29(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_30(uint start_byte_offset) {
  uint v_31 = (4u + start_byte_offset);
  uint v_32 = (8u + start_byte_offset);
  uint v_33 = (12u + start_byte_offset);
  uint v_34 = (16u + start_byte_offset);
  uint v_35 = (20u + start_byte_offset);
  uint v_36 = (24u + start_byte_offset);
  uint v_37 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_38 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_31 / 16u)][((v_31 & 15u) >> 2u)]), asfloat(arg_0_params[(v_32 / 16u)][((v_32 & 15u) >> 2u)]), asfloat(arg_0_params[(v_33 / 16u)][((v_33 & 15u) >> 2u)]), asfloat(arg_0_params[(v_34 / 16u)][((v_34 & 15u) >> 2u)]), asfloat(arg_0_params[(v_35 / 16u)][((v_35 & 15u) >> 2u)]), asfloat(arg_0_params[(v_36 / 16u)][((v_36 & 15u) >> 2u)]), asfloat(arg_0_params[(v_37 / 16u)][((v_37 & 15u) >> 2u)])};
  return v_38;
}

float3x4 v_39(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_40(uint start_byte_offset) {
  uint v_41 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_42 = (4u + start_byte_offset);
  uint v_43 = arg_0_params[(v_42 / 16u)][((v_42 & 15u) >> 2u)];
  float3x4 v_44 = v_39((16u + start_byte_offset));
  tint_TransferFunctionParams v_45 = v_30((64u + start_byte_offset));
  tint_TransferFunctionParams v_46 = v_30((96u + start_byte_offset));
  float3x3 v_47 = v_29((128u + start_byte_offset));
  float3x2 v_48 = v_21((176u + start_byte_offset));
  float3x2 v_49 = v_21((200u + start_byte_offset));
  uint v_50 = (224u + start_byte_offset);
  uint4 v_51 = arg_0_params[(v_50 / 16u)];
  float2 v_52 = asfloat((((((v_50 & 15u) >> 2u) == 2u)) ? (v_51.zw) : (v_51.xy)));
  uint v_53 = (232u + start_byte_offset);
  uint4 v_54 = arg_0_params[(v_53 / 16u)];
  float2 v_55 = asfloat((((((v_53 & 15u) >> 2u) == 2u)) ? (v_54.zw) : (v_54.xy)));
  uint v_56 = (240u + start_byte_offset);
  uint4 v_57 = arg_0_params[(v_56 / 16u)];
  float2 v_58 = asfloat((((((v_56 & 15u) >> 2u) == 2u)) ? (v_57.zw) : (v_57.xy)));
  uint v_59 = (248u + start_byte_offset);
  uint4 v_60 = arg_0_params[(v_59 / 16u)];
  float2 v_61 = asfloat((((((v_59 & 15u) >> 2u) == 2u)) ? (v_60.zw) : (v_60.xy)));
  uint v_62 = (256u + start_byte_offset);
  uint4 v_63 = arg_0_params[(v_62 / 16u)];
  uint2 v_64 = (((((v_62 & 15u) >> 2u) == 2u)) ? (v_63.zw) : (v_63.xy));
  uint v_65 = (264u + start_byte_offset);
  uint4 v_66 = arg_0_params[(v_65 / 16u)];
  tint_ExternalTextureParams v_67 = {v_41, v_43, v_44, v_45, v_46, v_47, v_48, v_49, v_52, v_55, v_58, v_61, v_64, asfloat((((((v_65 & 15u) >> 2u) == 2u)) ? (v_66.zw) : (v_66.xy)))};
  return v_67;
}

float4 textureSampleBaseClampToEdge_7c04e6() {
  tint_ExternalTextureParams v_68 = v_40(0u);
  float4 res = tint_TextureSampleClampToEdgeMultiplanarExternal(arg_0_plane0, arg_0_plane1, v_68, arg_1, (1.0f).xx);
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
  uint4 arg_0_params[17];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t2, space1);
SamplerState arg_1 : register(s1, space1);
float3 tint_ApplyGammaTransferFunction(float3 v, tint_TransferFunctionParams params) {
  float3 v_1 = float3((params.G).xxx);
  float3 v_2 = float3((params.D).xxx);
  float3 v_3 = abs(v);
  float3 v_4 = float3(sign(v));
  return (((v_3 < v_2)) ? ((v_4 * ((params.C * v_3) + params.F))) : ((v_4 * (pow(((params.A * v_3) + params.B), v_1) + params.E))));
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
    tint_TransferFunctionParams v_20 = params.dstTransferFunction;
    v_18 = tint_ApplyGammaTransferFunction(mul(tint_ApplySrcTransferFunction(v_17, v_19), params.gamutConversionMatrix), v_20);
  } else {
    v_18 = v_17;
  }
  return float4(v_18, v_15);
}

float3x2 v_21(uint start_byte_offset) {
  uint4 v_22 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_23 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_22.zw) : (v_22.xy)));
  uint v_24 = (8u + start_byte_offset);
  uint4 v_25 = arg_0_params[(v_24 / 16u)];
  float2 v_26 = asfloat((((((v_24 & 15u) >> 2u) == 2u)) ? (v_25.zw) : (v_25.xy)));
  uint v_27 = (16u + start_byte_offset);
  uint4 v_28 = arg_0_params[(v_27 / 16u)];
  return float3x2(v_23, v_26, asfloat((((((v_27 & 15u) >> 2u) == 2u)) ? (v_28.zw) : (v_28.xy))));
}

float3x3 v_29(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_30(uint start_byte_offset) {
  uint v_31 = (4u + start_byte_offset);
  uint v_32 = (8u + start_byte_offset);
  uint v_33 = (12u + start_byte_offset);
  uint v_34 = (16u + start_byte_offset);
  uint v_35 = (20u + start_byte_offset);
  uint v_36 = (24u + start_byte_offset);
  uint v_37 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_38 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_31 / 16u)][((v_31 & 15u) >> 2u)]), asfloat(arg_0_params[(v_32 / 16u)][((v_32 & 15u) >> 2u)]), asfloat(arg_0_params[(v_33 / 16u)][((v_33 & 15u) >> 2u)]), asfloat(arg_0_params[(v_34 / 16u)][((v_34 & 15u) >> 2u)]), asfloat(arg_0_params[(v_35 / 16u)][((v_35 & 15u) >> 2u)]), asfloat(arg_0_params[(v_36 / 16u)][((v_36 & 15u) >> 2u)]), asfloat(arg_0_params[(v_37 / 16u)][((v_37 & 15u) >> 2u)])};
  return v_38;
}

float3x4 v_39(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_40(uint start_byte_offset) {
  uint v_41 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_42 = (4u + start_byte_offset);
  uint v_43 = arg_0_params[(v_42 / 16u)][((v_42 & 15u) >> 2u)];
  float3x4 v_44 = v_39((16u + start_byte_offset));
  tint_TransferFunctionParams v_45 = v_30((64u + start_byte_offset));
  tint_TransferFunctionParams v_46 = v_30((96u + start_byte_offset));
  float3x3 v_47 = v_29((128u + start_byte_offset));
  float3x2 v_48 = v_21((176u + start_byte_offset));
  float3x2 v_49 = v_21((200u + start_byte_offset));
  uint v_50 = (224u + start_byte_offset);
  uint4 v_51 = arg_0_params[(v_50 / 16u)];
  float2 v_52 = asfloat((((((v_50 & 15u) >> 2u) == 2u)) ? (v_51.zw) : (v_51.xy)));
  uint v_53 = (232u + start_byte_offset);
  uint4 v_54 = arg_0_params[(v_53 / 16u)];
  float2 v_55 = asfloat((((((v_53 & 15u) >> 2u) == 2u)) ? (v_54.zw) : (v_54.xy)));
  uint v_56 = (240u + start_byte_offset);
  uint4 v_57 = arg_0_params[(v_56 / 16u)];
  float2 v_58 = asfloat((((((v_56 & 15u) >> 2u) == 2u)) ? (v_57.zw) : (v_57.xy)));
  uint v_59 = (248u + start_byte_offset);
  uint4 v_60 = arg_0_params[(v_59 / 16u)];
  float2 v_61 = asfloat((((((v_59 & 15u) >> 2u) == 2u)) ? (v_60.zw) : (v_60.xy)));
  uint v_62 = (256u + start_byte_offset);
  uint4 v_63 = arg_0_params[(v_62 / 16u)];
  uint2 v_64 = (((((v_62 & 15u) >> 2u) == 2u)) ? (v_63.zw) : (v_63.xy));
  uint v_65 = (264u + start_byte_offset);
  uint4 v_66 = arg_0_params[(v_65 / 16u)];
  tint_ExternalTextureParams v_67 = {v_41, v_43, v_44, v_45, v_46, v_47, v_48, v_49, v_52, v_55, v_58, v_61, v_64, asfloat((((((v_65 & 15u) >> 2u) == 2u)) ? (v_66.zw) : (v_66.xy)))};
  return v_67;
}

float4 textureSampleBaseClampToEdge_7c04e6() {
  tint_ExternalTextureParams v_68 = v_40(0u);
  float4 res = tint_TextureSampleClampToEdgeMultiplanarExternal(arg_0_plane0, arg_0_plane1, v_68, arg_1, (1.0f).xx);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_69 = (VertexOutput)0;
  v_69.pos = (0.0f).xxxx;
  v_69.prevent_dce = textureSampleBaseClampToEdge_7c04e6();
  VertexOutput v_70 = v_69;
  return v_70;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_71 = vertex_main_inner();
  vertex_main_outputs v_72 = {v_71.prevent_dce, v_71.pos};
  return v_72;
}

