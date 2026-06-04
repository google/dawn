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
cbuffer cbuffer_arg_0_params : register(b2, space1) {
  uint4 arg_0_params[17];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t1, space1);
uint2 tint_v2f32_to_v2u32(float2 value) {
  return uint2(clamp(value, (0.0f).xx, (4294967040.0f).xx));
}

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
    tint_TransferFunctionParams v_24 = params.dstTransferFunction;
    v_22 = tint_ApplyGammaTransferFunction(mul(tint_ApplySrcTransferFunction(v_21, v_23), params.gamutConversionMatrix), v_24);
  } else {
    v_22 = v_21;
  }
  return float4(v_22, v_15);
}

float3x2 v_25(uint start_byte_offset) {
  uint4 v_26 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_27 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_26.zw) : (v_26.xy)));
  uint v_28 = (8u + start_byte_offset);
  uint4 v_29 = arg_0_params[(v_28 / 16u)];
  float2 v_30 = asfloat((((((v_28 & 15u) >> 2u) == 2u)) ? (v_29.zw) : (v_29.xy)));
  uint v_31 = (16u + start_byte_offset);
  uint4 v_32 = arg_0_params[(v_31 / 16u)];
  return float3x2(v_27, v_30, asfloat((((((v_31 & 15u) >> 2u) == 2u)) ? (v_32.zw) : (v_32.xy))));
}

float3x3 v_33(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_34(uint start_byte_offset) {
  uint v_35 = (4u + start_byte_offset);
  uint v_36 = (8u + start_byte_offset);
  uint v_37 = (12u + start_byte_offset);
  uint v_38 = (16u + start_byte_offset);
  uint v_39 = (20u + start_byte_offset);
  uint v_40 = (24u + start_byte_offset);
  uint v_41 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_42 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_35 / 16u)][((v_35 & 15u) >> 2u)]), asfloat(arg_0_params[(v_36 / 16u)][((v_36 & 15u) >> 2u)]), asfloat(arg_0_params[(v_37 / 16u)][((v_37 & 15u) >> 2u)]), asfloat(arg_0_params[(v_38 / 16u)][((v_38 & 15u) >> 2u)]), asfloat(arg_0_params[(v_39 / 16u)][((v_39 & 15u) >> 2u)]), asfloat(arg_0_params[(v_40 / 16u)][((v_40 & 15u) >> 2u)]), asfloat(arg_0_params[(v_41 / 16u)][((v_41 & 15u) >> 2u)])};
  return v_42;
}

float3x4 v_43(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_44(uint start_byte_offset) {
  uint v_45 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_46 = (4u + start_byte_offset);
  uint v_47 = arg_0_params[(v_46 / 16u)][((v_46 & 15u) >> 2u)];
  float3x4 v_48 = v_43((16u + start_byte_offset));
  tint_TransferFunctionParams v_49 = v_34((64u + start_byte_offset));
  tint_TransferFunctionParams v_50 = v_34((96u + start_byte_offset));
  float3x3 v_51 = v_33((128u + start_byte_offset));
  float3x2 v_52 = v_25((176u + start_byte_offset));
  float3x2 v_53 = v_25((200u + start_byte_offset));
  uint v_54 = (224u + start_byte_offset);
  uint4 v_55 = arg_0_params[(v_54 / 16u)];
  float2 v_56 = asfloat((((((v_54 & 15u) >> 2u) == 2u)) ? (v_55.zw) : (v_55.xy)));
  uint v_57 = (232u + start_byte_offset);
  uint4 v_58 = arg_0_params[(v_57 / 16u)];
  float2 v_59 = asfloat((((((v_57 & 15u) >> 2u) == 2u)) ? (v_58.zw) : (v_58.xy)));
  uint v_60 = (240u + start_byte_offset);
  uint4 v_61 = arg_0_params[(v_60 / 16u)];
  float2 v_62 = asfloat((((((v_60 & 15u) >> 2u) == 2u)) ? (v_61.zw) : (v_61.xy)));
  uint v_63 = (248u + start_byte_offset);
  uint4 v_64 = arg_0_params[(v_63 / 16u)];
  float2 v_65 = asfloat((((((v_63 & 15u) >> 2u) == 2u)) ? (v_64.zw) : (v_64.xy)));
  uint v_66 = (256u + start_byte_offset);
  uint4 v_67 = arg_0_params[(v_66 / 16u)];
  uint2 v_68 = (((((v_66 & 15u) >> 2u) == 2u)) ? (v_67.zw) : (v_67.xy));
  uint v_69 = (264u + start_byte_offset);
  uint4 v_70 = arg_0_params[(v_69 / 16u)];
  tint_ExternalTextureParams v_71 = {v_45, v_47, v_48, v_49, v_50, v_51, v_52, v_53, v_56, v_59, v_62, v_65, v_68, asfloat((((((v_69 & 15u) >> 2u) == 2u)) ? (v_70.zw) : (v_70.xy)))};
  return v_71;
}

float4 textureLoad_1bfdfb() {
  tint_ExternalTextureParams v_72 = v_44(0u);
  float4 res = tint_TextureLoadMultiplanarExternal(arg_0_plane0, arg_0_plane1, v_72, (1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_1bfdfb()));
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
cbuffer cbuffer_arg_0_params : register(b2, space1) {
  uint4 arg_0_params[17];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t1, space1);
uint2 tint_v2f32_to_v2u32(float2 value) {
  return uint2(clamp(value, (0.0f).xx, (4294967040.0f).xx));
}

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
    tint_TransferFunctionParams v_24 = params.dstTransferFunction;
    v_22 = tint_ApplyGammaTransferFunction(mul(tint_ApplySrcTransferFunction(v_21, v_23), params.gamutConversionMatrix), v_24);
  } else {
    v_22 = v_21;
  }
  return float4(v_22, v_15);
}

float3x2 v_25(uint start_byte_offset) {
  uint4 v_26 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_27 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_26.zw) : (v_26.xy)));
  uint v_28 = (8u + start_byte_offset);
  uint4 v_29 = arg_0_params[(v_28 / 16u)];
  float2 v_30 = asfloat((((((v_28 & 15u) >> 2u) == 2u)) ? (v_29.zw) : (v_29.xy)));
  uint v_31 = (16u + start_byte_offset);
  uint4 v_32 = arg_0_params[(v_31 / 16u)];
  return float3x2(v_27, v_30, asfloat((((((v_31 & 15u) >> 2u) == 2u)) ? (v_32.zw) : (v_32.xy))));
}

float3x3 v_33(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_34(uint start_byte_offset) {
  uint v_35 = (4u + start_byte_offset);
  uint v_36 = (8u + start_byte_offset);
  uint v_37 = (12u + start_byte_offset);
  uint v_38 = (16u + start_byte_offset);
  uint v_39 = (20u + start_byte_offset);
  uint v_40 = (24u + start_byte_offset);
  uint v_41 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_42 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_35 / 16u)][((v_35 & 15u) >> 2u)]), asfloat(arg_0_params[(v_36 / 16u)][((v_36 & 15u) >> 2u)]), asfloat(arg_0_params[(v_37 / 16u)][((v_37 & 15u) >> 2u)]), asfloat(arg_0_params[(v_38 / 16u)][((v_38 & 15u) >> 2u)]), asfloat(arg_0_params[(v_39 / 16u)][((v_39 & 15u) >> 2u)]), asfloat(arg_0_params[(v_40 / 16u)][((v_40 & 15u) >> 2u)]), asfloat(arg_0_params[(v_41 / 16u)][((v_41 & 15u) >> 2u)])};
  return v_42;
}

float3x4 v_43(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_44(uint start_byte_offset) {
  uint v_45 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_46 = (4u + start_byte_offset);
  uint v_47 = arg_0_params[(v_46 / 16u)][((v_46 & 15u) >> 2u)];
  float3x4 v_48 = v_43((16u + start_byte_offset));
  tint_TransferFunctionParams v_49 = v_34((64u + start_byte_offset));
  tint_TransferFunctionParams v_50 = v_34((96u + start_byte_offset));
  float3x3 v_51 = v_33((128u + start_byte_offset));
  float3x2 v_52 = v_25((176u + start_byte_offset));
  float3x2 v_53 = v_25((200u + start_byte_offset));
  uint v_54 = (224u + start_byte_offset);
  uint4 v_55 = arg_0_params[(v_54 / 16u)];
  float2 v_56 = asfloat((((((v_54 & 15u) >> 2u) == 2u)) ? (v_55.zw) : (v_55.xy)));
  uint v_57 = (232u + start_byte_offset);
  uint4 v_58 = arg_0_params[(v_57 / 16u)];
  float2 v_59 = asfloat((((((v_57 & 15u) >> 2u) == 2u)) ? (v_58.zw) : (v_58.xy)));
  uint v_60 = (240u + start_byte_offset);
  uint4 v_61 = arg_0_params[(v_60 / 16u)];
  float2 v_62 = asfloat((((((v_60 & 15u) >> 2u) == 2u)) ? (v_61.zw) : (v_61.xy)));
  uint v_63 = (248u + start_byte_offset);
  uint4 v_64 = arg_0_params[(v_63 / 16u)];
  float2 v_65 = asfloat((((((v_63 & 15u) >> 2u) == 2u)) ? (v_64.zw) : (v_64.xy)));
  uint v_66 = (256u + start_byte_offset);
  uint4 v_67 = arg_0_params[(v_66 / 16u)];
  uint2 v_68 = (((((v_66 & 15u) >> 2u) == 2u)) ? (v_67.zw) : (v_67.xy));
  uint v_69 = (264u + start_byte_offset);
  uint4 v_70 = arg_0_params[(v_69 / 16u)];
  tint_ExternalTextureParams v_71 = {v_45, v_47, v_48, v_49, v_50, v_51, v_52, v_53, v_56, v_59, v_62, v_65, v_68, asfloat((((((v_69 & 15u) >> 2u) == 2u)) ? (v_70.zw) : (v_70.xy)))};
  return v_71;
}

float4 textureLoad_1bfdfb() {
  tint_ExternalTextureParams v_72 = v_44(0u);
  float4 res = tint_TextureLoadMultiplanarExternal(arg_0_plane0, arg_0_plane1, v_72, (1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_1bfdfb()));
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


cbuffer cbuffer_arg_0_params : register(b2, space1) {
  uint4 arg_0_params[17];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t1, space1);
uint2 tint_v2f32_to_v2u32(float2 value) {
  return uint2(clamp(value, (0.0f).xx, (4294967040.0f).xx));
}

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
    tint_TransferFunctionParams v_24 = params.dstTransferFunction;
    v_22 = tint_ApplyGammaTransferFunction(mul(tint_ApplySrcTransferFunction(v_21, v_23), params.gamutConversionMatrix), v_24);
  } else {
    v_22 = v_21;
  }
  return float4(v_22, v_15);
}

float3x2 v_25(uint start_byte_offset) {
  uint4 v_26 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_27 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_26.zw) : (v_26.xy)));
  uint v_28 = (8u + start_byte_offset);
  uint4 v_29 = arg_0_params[(v_28 / 16u)];
  float2 v_30 = asfloat((((((v_28 & 15u) >> 2u) == 2u)) ? (v_29.zw) : (v_29.xy)));
  uint v_31 = (16u + start_byte_offset);
  uint4 v_32 = arg_0_params[(v_31 / 16u)];
  return float3x2(v_27, v_30, asfloat((((((v_31 & 15u) >> 2u) == 2u)) ? (v_32.zw) : (v_32.xy))));
}

float3x3 v_33(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_34(uint start_byte_offset) {
  uint v_35 = (4u + start_byte_offset);
  uint v_36 = (8u + start_byte_offset);
  uint v_37 = (12u + start_byte_offset);
  uint v_38 = (16u + start_byte_offset);
  uint v_39 = (20u + start_byte_offset);
  uint v_40 = (24u + start_byte_offset);
  uint v_41 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_42 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_35 / 16u)][((v_35 & 15u) >> 2u)]), asfloat(arg_0_params[(v_36 / 16u)][((v_36 & 15u) >> 2u)]), asfloat(arg_0_params[(v_37 / 16u)][((v_37 & 15u) >> 2u)]), asfloat(arg_0_params[(v_38 / 16u)][((v_38 & 15u) >> 2u)]), asfloat(arg_0_params[(v_39 / 16u)][((v_39 & 15u) >> 2u)]), asfloat(arg_0_params[(v_40 / 16u)][((v_40 & 15u) >> 2u)]), asfloat(arg_0_params[(v_41 / 16u)][((v_41 & 15u) >> 2u)])};
  return v_42;
}

float3x4 v_43(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_44(uint start_byte_offset) {
  uint v_45 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_46 = (4u + start_byte_offset);
  uint v_47 = arg_0_params[(v_46 / 16u)][((v_46 & 15u) >> 2u)];
  float3x4 v_48 = v_43((16u + start_byte_offset));
  tint_TransferFunctionParams v_49 = v_34((64u + start_byte_offset));
  tint_TransferFunctionParams v_50 = v_34((96u + start_byte_offset));
  float3x3 v_51 = v_33((128u + start_byte_offset));
  float3x2 v_52 = v_25((176u + start_byte_offset));
  float3x2 v_53 = v_25((200u + start_byte_offset));
  uint v_54 = (224u + start_byte_offset);
  uint4 v_55 = arg_0_params[(v_54 / 16u)];
  float2 v_56 = asfloat((((((v_54 & 15u) >> 2u) == 2u)) ? (v_55.zw) : (v_55.xy)));
  uint v_57 = (232u + start_byte_offset);
  uint4 v_58 = arg_0_params[(v_57 / 16u)];
  float2 v_59 = asfloat((((((v_57 & 15u) >> 2u) == 2u)) ? (v_58.zw) : (v_58.xy)));
  uint v_60 = (240u + start_byte_offset);
  uint4 v_61 = arg_0_params[(v_60 / 16u)];
  float2 v_62 = asfloat((((((v_60 & 15u) >> 2u) == 2u)) ? (v_61.zw) : (v_61.xy)));
  uint v_63 = (248u + start_byte_offset);
  uint4 v_64 = arg_0_params[(v_63 / 16u)];
  float2 v_65 = asfloat((((((v_63 & 15u) >> 2u) == 2u)) ? (v_64.zw) : (v_64.xy)));
  uint v_66 = (256u + start_byte_offset);
  uint4 v_67 = arg_0_params[(v_66 / 16u)];
  uint2 v_68 = (((((v_66 & 15u) >> 2u) == 2u)) ? (v_67.zw) : (v_67.xy));
  uint v_69 = (264u + start_byte_offset);
  uint4 v_70 = arg_0_params[(v_69 / 16u)];
  tint_ExternalTextureParams v_71 = {v_45, v_47, v_48, v_49, v_50, v_51, v_52, v_53, v_56, v_59, v_62, v_65, v_68, asfloat((((((v_69 & 15u) >> 2u) == 2u)) ? (v_70.zw) : (v_70.xy)))};
  return v_71;
}

float4 textureLoad_1bfdfb() {
  tint_ExternalTextureParams v_72 = v_44(0u);
  float4 res = tint_TextureLoadMultiplanarExternal(arg_0_plane0, arg_0_plane1, v_72, (1u).xx);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_73 = (VertexOutput)0;
  v_73.pos = (0.0f).xxxx;
  v_73.prevent_dce = textureLoad_1bfdfb();
  VertexOutput v_74 = v_73;
  return v_74;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_75 = vertex_main_inner();
  vertex_main_outputs v_76 = {v_75.prevent_dce, v_75.pos};
  return v_76;
}

