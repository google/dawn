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

struct vertex_main_outputs {
  float4 tint_symbol_1 : SV_Position;
};


Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t1, space1);
cbuffer cbuffer_arg_0_params : register(b2, space1) {
  uint4 arg_0_params[17];
};
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
    int2 v_9 = int2(v_6);
    float4 v_10 = float4(plane_0.Load(int3(v_9, int(0u))));
    v_7 = v_10.xyz;
    v_8 = v_10[3u];
  } else {
    int2 v_11 = int2(v_6);
    float v_12 = float4(plane_0.Load(int3(v_11, int(0u))))[0u];
    int2 v_13 = int2(tint_v2f32_to_v2u32((v_5 * params.plane1CoordFactor)));
    v_7 = mul(params.yuvToRgbConversionMatrix, float4(v_12, float4(plane_1.Load(int3(v_13, int(0u)))).xy, 1.0f));
    v_8 = 1.0f;
  }
  float3 v_14 = v_7;
  float3 v_15 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_GammaTransferParams v_16 = params.gammaDecodeParams;
    tint_GammaTransferParams v_17 = params.gammaEncodeParams;
    v_15 = tint_GammaCorrection(mul(tint_GammaCorrection(v_14, v_16), params.gamutConversionMatrix), v_17);
  } else {
    v_15 = v_14;
  }
  return float4(v_15, v_8);
}

float4 textureLoad2d(Texture2D<float4> tint_symbol_plane0, Texture2D<float4> tint_symbol_plane1, tint_ExternalTextureParams tint_symbol_params, int2 coords) {
  return tint_TextureLoadExternal(tint_symbol_plane0, tint_symbol_plane1, tint_symbol_params, uint2(coords));
}

float3x2 v_18(uint start_byte_offset) {
  uint4 v_19 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_20 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_19.zw) : (v_19.xy)));
  uint4 v_21 = arg_0_params[((8u + start_byte_offset) / 16u)];
  float2 v_22 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_21.zw) : (v_21.xy)));
  uint4 v_23 = arg_0_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_20, v_22, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_23.zw) : (v_23.xy))));
}

float3x3 v_24(uint start_byte_offset) {
  float3 v_25 = asfloat(arg_0_params[(start_byte_offset / 16u)].xyz);
  float3 v_26 = asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_25, v_26, asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_27(uint start_byte_offset) {
  float v_28 = asfloat(arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float v_29 = asfloat(arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  float v_30 = asfloat(arg_0_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  float v_31 = asfloat(arg_0_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]);
  float v_32 = asfloat(arg_0_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]);
  float v_33 = asfloat(arg_0_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]);
  float v_34 = asfloat(arg_0_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]);
  tint_GammaTransferParams v_35 = {v_28, v_29, v_30, v_31, v_32, v_33, v_34, arg_0_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_35;
}

float3x4 v_36(uint start_byte_offset) {
  float4 v_37 = asfloat(arg_0_params[(start_byte_offset / 16u)]);
  float4 v_38 = asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_37, v_38, asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_39(uint start_byte_offset) {
  uint v_40 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_41 = arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_42 = v_36((16u + start_byte_offset));
  tint_GammaTransferParams v_43 = v_27((64u + start_byte_offset));
  tint_GammaTransferParams v_44 = v_27((96u + start_byte_offset));
  float3x3 v_45 = v_24((128u + start_byte_offset));
  float3x2 v_46 = v_18((176u + start_byte_offset));
  float3x2 v_47 = v_18((200u + start_byte_offset));
  uint4 v_48 = arg_0_params[((224u + start_byte_offset) / 16u)];
  float2 v_49 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_48.zw) : (v_48.xy)));
  uint4 v_50 = arg_0_params[((232u + start_byte_offset) / 16u)];
  float2 v_51 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_50.zw) : (v_50.xy)));
  uint4 v_52 = arg_0_params[((240u + start_byte_offset) / 16u)];
  float2 v_53 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_52.zw) : (v_52.xy)));
  uint4 v_54 = arg_0_params[((248u + start_byte_offset) / 16u)];
  float2 v_55 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_54.zw) : (v_54.xy)));
  uint4 v_56 = arg_0_params[((256u + start_byte_offset) / 16u)];
  uint2 v_57 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_56.zw) : (v_56.xy));
  uint4 v_58 = arg_0_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_59 = {v_40, v_41, v_42, v_43, v_44, v_45, v_46, v_47, v_49, v_51, v_53, v_55, v_57, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_58.zw) : (v_58.xy)))};
  return v_59;
}

void doTextureLoad() {
  tint_ExternalTextureParams v_60 = v_39(0u);
  float4 res = textureLoad2d(arg_0_plane0, arg_0_plane1, v_60, (int(0)).xx);
}

float4 vertex_main_inner() {
  doTextureLoad();
  return (0.0f).xxxx;
}

void fragment_main() {
  doTextureLoad();
}

[numthreads(1, 1, 1)]
void compute_main() {
  doTextureLoad();
}

vertex_main_outputs vertex_main() {
  vertex_main_outputs v_61 = {vertex_main_inner()};
  return v_61;
}

