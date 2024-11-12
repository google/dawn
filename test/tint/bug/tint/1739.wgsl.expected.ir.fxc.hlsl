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
  float3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  float3x3 gamutConversionMatrix;
  float3x2 sampleTransform;
  float3x2 loadTransform;
  float2 samplePlane0RectMin;
  float2 samplePlane0RectMax;
  float2 samplePlane1RectMin;
  float2 samplePlane1RectMax;
  uint2 visibleSize;
  float2 plane1CoordFactor;
};


Texture2D<float4> t_plane0 : register(t0);
Texture2D<float4> t_plane1 : register(t2);
cbuffer cbuffer_t_params : register(b3) {
  uint4 t_params[17];
};
RWTexture2D<float4> outImage : register(u1);
uint2 tint_v2f32_to_v2u32(float2 value) {
  return (((value <= (4294967040.0f).xx)) ? ((((value >= (0.0f).xx)) ? (uint2(value)) : ((0u).xx))) : ((4294967295u).xx));
}

float3 tint_GammaCorrection(float3 v, tint_GammaTransferParams params) {
  float3 v_1 = float3((params.G).xxx);
  float3 v_2 = float3((params.D).xxx);
  float3 v_3 = abs(v);
  float3 v_4 = float3(sign(v));
  return (((v_3 < v_2)) ? ((v_4 * ((params.C * v_3) + params.F))) : ((v_4 * (pow(((params.A * v_3) + params.B), v_1) + params.E))));
}

float4 tint_TextureLoadExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, uint2 coords) {
  float2 v_5 = round(mul(float3(float2(min(coords, params.visibleSize)), 1.0f), params.loadTransform));
  uint2 v_6 = tint_v2f32_to_v2u32(v_5);
  float3 v_7 = (0.0f).xxx;
  float v_8 = 0.0f;
  if ((params.numPlanes == 1u)) {
    uint3 v_9 = (0u).xxx;
    plane_0.GetDimensions(0u, v_9[0u], v_9[1u], v_9[2u]);
    uint v_10 = min(0u, (v_9.z - 1u));
    uint3 v_11 = (0u).xxx;
    plane_0.GetDimensions(uint(v_10), v_11[0u], v_11[1u], v_11[2u]);
    int2 v_12 = int2(min(v_6, (v_11.xy - (1u).xx)));
    float4 v_13 = float4(plane_0.Load(int3(v_12, int(v_10))));
    v_7 = v_13.xyz;
    v_8 = v_13[3u];
  } else {
    uint3 v_14 = (0u).xxx;
    plane_0.GetDimensions(0u, v_14[0u], v_14[1u], v_14[2u]);
    uint v_15 = min(0u, (v_14.z - 1u));
    uint3 v_16 = (0u).xxx;
    plane_0.GetDimensions(uint(v_15), v_16[0u], v_16[1u], v_16[2u]);
    int2 v_17 = int2(min(v_6, (v_16.xy - (1u).xx)));
    float v_18 = float4(plane_0.Load(int3(v_17, int(v_15))))[0u];
    uint2 v_19 = tint_v2f32_to_v2u32((v_5 * params.plane1CoordFactor));
    uint3 v_20 = (0u).xxx;
    plane_1.GetDimensions(0u, v_20[0u], v_20[1u], v_20[2u]);
    uint v_21 = min(0u, (v_20.z - 1u));
    uint3 v_22 = (0u).xxx;
    plane_1.GetDimensions(uint(v_21), v_22[0u], v_22[1u], v_22[2u]);
    int2 v_23 = int2(min(v_19, (v_22.xy - (1u).xx)));
    v_7 = mul(params.yuvToRgbConversionMatrix, float4(v_18, float4(plane_1.Load(int3(v_23, int(v_21)))).xy, 1.0f));
    v_8 = 1.0f;
  }
  float3 v_24 = v_7;
  float3 v_25 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_GammaTransferParams v_26 = params.gammaDecodeParams;
    tint_GammaTransferParams v_27 = params.gammaEncodeParams;
    v_25 = tint_GammaCorrection(mul(tint_GammaCorrection(v_24, v_26), params.gamutConversionMatrix), v_27);
  } else {
    v_25 = v_24;
  }
  return float4(v_25, v_8);
}

float3x2 v_28(uint start_byte_offset) {
  uint4 v_29 = t_params[(start_byte_offset / 16u)];
  float2 v_30 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_29.zw) : (v_29.xy)));
  uint4 v_31 = t_params[((8u + start_byte_offset) / 16u)];
  float2 v_32 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_31.zw) : (v_31.xy)));
  uint4 v_33 = t_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_30, v_32, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_33.zw) : (v_33.xy))));
}

float3x3 v_34(uint start_byte_offset) {
  float3 v_35 = asfloat(t_params[(start_byte_offset / 16u)].xyz);
  float3 v_36 = asfloat(t_params[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_35, v_36, asfloat(t_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_37(uint start_byte_offset) {
  float v_38 = asfloat(t_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float v_39 = asfloat(t_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  float v_40 = asfloat(t_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  float v_41 = asfloat(t_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]);
  float v_42 = asfloat(t_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]);
  float v_43 = asfloat(t_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]);
  float v_44 = asfloat(t_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]);
  tint_GammaTransferParams v_45 = {v_38, v_39, v_40, v_41, v_42, v_43, v_44, t_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_45;
}

float3x4 v_46(uint start_byte_offset) {
  float4 v_47 = asfloat(t_params[(start_byte_offset / 16u)]);
  float4 v_48 = asfloat(t_params[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_47, v_48, asfloat(t_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_49(uint start_byte_offset) {
  uint v_50 = t_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_51 = t_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_52 = v_46((16u + start_byte_offset));
  tint_GammaTransferParams v_53 = v_37((64u + start_byte_offset));
  tint_GammaTransferParams v_54 = v_37((96u + start_byte_offset));
  float3x3 v_55 = v_34((128u + start_byte_offset));
  float3x2 v_56 = v_28((176u + start_byte_offset));
  float3x2 v_57 = v_28((200u + start_byte_offset));
  uint4 v_58 = t_params[((224u + start_byte_offset) / 16u)];
  float2 v_59 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_58.zw) : (v_58.xy)));
  uint4 v_60 = t_params[((232u + start_byte_offset) / 16u)];
  float2 v_61 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_60.zw) : (v_60.xy)));
  uint4 v_62 = t_params[((240u + start_byte_offset) / 16u)];
  float2 v_63 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_62.zw) : (v_62.xy)));
  uint4 v_64 = t_params[((248u + start_byte_offset) / 16u)];
  float2 v_65 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_64.zw) : (v_64.xy)));
  uint4 v_66 = t_params[((256u + start_byte_offset) / 16u)];
  uint2 v_67 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_66.zw) : (v_66.xy));
  uint4 v_68 = t_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_69 = {v_50, v_51, v_52, v_53, v_54, v_55, v_56, v_57, v_59, v_61, v_63, v_65, v_67, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_68.zw) : (v_68.xy)))};
  return v_69;
}

[numthreads(1, 1, 1)]
void main() {
  tint_ExternalTextureParams v_70 = v_49(0u);
  float4 red = tint_TextureLoadExternal(t_plane0, t_plane1, v_70, uint2((int(10)).xx));
  outImage[(int(0)).xx] = red;
  tint_ExternalTextureParams v_71 = v_49(0u);
  float4 green = tint_TextureLoadExternal(t_plane0, t_plane1, v_71, uint2(int2(int(70), int(118))));
  outImage[int2(int(1), int(0))] = green;
}

