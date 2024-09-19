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
    uint2 v_9 = (0u).xx;
    plane_0.GetDimensions(v_9[0u], v_9[1u]);
    uint2 v_10 = min(v_6, (v_9 - (1u).xx));
    uint3 v_11 = (0u).xxx;
    plane_0.GetDimensions(0u, v_11[0u], v_11[1u], v_11[2u]);
    uint v_12 = min(0u, (v_11.z - 1u));
    int2 v_13 = int2(v_10);
    float4 v_14 = float4(plane_0.Load(int3(v_13, int(v_12))));
    v_7 = v_14.xyz;
    v_8 = v_14[3u];
  } else {
    uint2 v_15 = (0u).xx;
    plane_0.GetDimensions(v_15[0u], v_15[1u]);
    uint2 v_16 = min(v_6, (v_15 - (1u).xx));
    uint3 v_17 = (0u).xxx;
    plane_0.GetDimensions(0u, v_17[0u], v_17[1u], v_17[2u]);
    uint v_18 = min(0u, (v_17.z - 1u));
    int2 v_19 = int2(v_16);
    float v_20 = float4(plane_0.Load(int3(v_19, int(v_18))))[0u];
    uint2 v_21 = tint_v2f32_to_v2u32((v_5 * params.plane1CoordFactor));
    uint2 v_22 = (0u).xx;
    plane_1.GetDimensions(v_22[0u], v_22[1u]);
    uint2 v_23 = min(v_21, (v_22 - (1u).xx));
    uint3 v_24 = (0u).xxx;
    plane_1.GetDimensions(0u, v_24[0u], v_24[1u], v_24[2u]);
    uint v_25 = min(0u, (v_24.z - 1u));
    int2 v_26 = int2(v_23);
    v_7 = mul(params.yuvToRgbConversionMatrix, float4(v_20, float4(plane_1.Load(int3(v_26, int(v_25)))).xy, 1.0f));
    v_8 = 1.0f;
  }
  float3 v_27 = v_7;
  float3 v_28 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_GammaTransferParams v_29 = params.gammaDecodeParams;
    tint_GammaTransferParams v_30 = params.gammaEncodeParams;
    v_28 = tint_GammaCorrection(mul(tint_GammaCorrection(v_27, v_29), params.gamutConversionMatrix), v_30);
  } else {
    v_28 = v_27;
  }
  return float4(v_28, v_8);
}

float3x2 v_31(uint start_byte_offset) {
  uint4 v_32 = t_params[(start_byte_offset / 16u)];
  float2 v_33 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_32.zw) : (v_32.xy)));
  uint4 v_34 = t_params[((8u + start_byte_offset) / 16u)];
  float2 v_35 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_34.zw) : (v_34.xy)));
  uint4 v_36 = t_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_33, v_35, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_36.zw) : (v_36.xy))));
}

float3x3 v_37(uint start_byte_offset) {
  float3 v_38 = asfloat(t_params[(start_byte_offset / 16u)].xyz);
  float3 v_39 = asfloat(t_params[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_38, v_39, asfloat(t_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_40(uint start_byte_offset) {
  float v_41 = asfloat(t_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float v_42 = asfloat(t_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  float v_43 = asfloat(t_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  float v_44 = asfloat(t_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]);
  float v_45 = asfloat(t_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]);
  float v_46 = asfloat(t_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]);
  float v_47 = asfloat(t_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]);
  tint_GammaTransferParams v_48 = {v_41, v_42, v_43, v_44, v_45, v_46, v_47, t_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_48;
}

float3x4 v_49(uint start_byte_offset) {
  float4 v_50 = asfloat(t_params[(start_byte_offset / 16u)]);
  float4 v_51 = asfloat(t_params[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_50, v_51, asfloat(t_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_52(uint start_byte_offset) {
  uint v_53 = t_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_54 = t_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_55 = v_49((16u + start_byte_offset));
  tint_GammaTransferParams v_56 = v_40((64u + start_byte_offset));
  tint_GammaTransferParams v_57 = v_40((96u + start_byte_offset));
  float3x3 v_58 = v_37((128u + start_byte_offset));
  float3x2 v_59 = v_31((176u + start_byte_offset));
  float3x2 v_60 = v_31((200u + start_byte_offset));
  uint4 v_61 = t_params[((224u + start_byte_offset) / 16u)];
  float2 v_62 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_61.zw) : (v_61.xy)));
  uint4 v_63 = t_params[((232u + start_byte_offset) / 16u)];
  float2 v_64 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_63.zw) : (v_63.xy)));
  uint4 v_65 = t_params[((240u + start_byte_offset) / 16u)];
  float2 v_66 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_65.zw) : (v_65.xy)));
  uint4 v_67 = t_params[((248u + start_byte_offset) / 16u)];
  float2 v_68 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_67.zw) : (v_67.xy)));
  uint4 v_69 = t_params[((256u + start_byte_offset) / 16u)];
  uint2 v_70 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_69.zw) : (v_69.xy));
  uint4 v_71 = t_params[((264u + start_byte_offset) / 16u)];
  tint_GammaTransferParams v_72 = v_56;
  tint_GammaTransferParams v_73 = v_57;
  tint_ExternalTextureParams v_74 = {v_53, v_54, v_55, v_72, v_73, v_58, v_59, v_60, v_62, v_64, v_66, v_68, v_70, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_71.zw) : (v_71.xy)))};
  return v_74;
}

[numthreads(1, 1, 1)]
void main() {
  tint_ExternalTextureParams v_75 = v_52(0u);
  tint_ExternalTextureParams v_76 = v_75;
  float4 red = tint_TextureLoadExternal(t_plane0, t_plane1, v_76, uint2((int(10)).xx));
  outImage[(int(0)).xx] = red;
  tint_ExternalTextureParams v_77 = v_52(0u);
  tint_ExternalTextureParams v_78 = v_77;
  float4 green = tint_TextureLoadExternal(t_plane0, t_plane1, v_78, uint2(int2(int(70), int(118))));
  outImage[int2(int(1), int(0))] = green;
}

