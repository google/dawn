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


RWByteAddressBuffer prevent_dce : register(u0);
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
  float3 v_3 = float3(sign(v));
  return (((abs(v) < v_2)) ? ((v_3 * ((params.C * abs(v)) + params.F))) : ((v_3 * (pow(((params.A * abs(v)) + params.B), v_1) + params.E))));
}

float4 tint_TextureLoadExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, uint2 coords) {
  float2 v_4 = round(mul(float3(float2(min(coords, params.apparentSize)), 1.0f), params.loadTransform));
  uint2 v_5 = tint_v2f32_to_v2u32(v_4);
  float3 v_6 = (0.0f).xxx;
  float v_7 = 0.0f;
  if ((params.numPlanes == 1u)) {
    uint3 v_8 = (0u).xxx;
    plane_0.GetDimensions(0u, v_8.x, v_8.y, v_8.z);
    uint3 v_9 = (0u).xxx;
    plane_0.GetDimensions(uint(min(0u, (v_8.z - 1u))), v_9.x, v_9.y, v_9.z);
    int2 v_10 = int2(min(v_5, (v_9.xy - (1u).xx)));
    float4 v_11 = float4(plane_0.Load(int3(v_10, int(min(0u, (v_8.z - 1u))))));
    v_6 = v_11.xyz;
    v_7 = v_11.w;
  } else {
    uint3 v_12 = (0u).xxx;
    plane_0.GetDimensions(0u, v_12.x, v_12.y, v_12.z);
    uint3 v_13 = (0u).xxx;
    plane_0.GetDimensions(uint(min(0u, (v_12.z - 1u))), v_13.x, v_13.y, v_13.z);
    int2 v_14 = int2(min(v_5, (v_13.xy - (1u).xx)));
    float v_15 = float4(plane_0.Load(int3(v_14, int(min(0u, (v_12.z - 1u)))))).x;
    uint2 v_16 = tint_v2f32_to_v2u32((v_4 * params.plane1CoordFactor));
    uint3 v_17 = (0u).xxx;
    plane_1.GetDimensions(0u, v_17.x, v_17.y, v_17.z);
    uint3 v_18 = (0u).xxx;
    plane_1.GetDimensions(uint(min(0u, (v_17.z - 1u))), v_18.x, v_18.y, v_18.z);
    int2 v_19 = int2(min(v_16, (v_18.xy - (1u).xx)));
    v_6 = mul(params.yuvToRgbConversionMatrix, float4(v_15, float4(plane_1.Load(int3(v_19, int(min(0u, (v_17.z - 1u)))))).xy, 1.0f));
    v_7 = 1.0f;
  }
  float3 v_20 = v_6;
  float3 v_21 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_GammaTransferParams v_22 = params.gammaDecodeParams;
    tint_GammaTransferParams v_23 = params.gammaEncodeParams;
    v_21 = tint_GammaCorrection(mul(tint_GammaCorrection(v_20, v_22), params.gamutConversionMatrix), v_23);
  } else {
    v_21 = v_20;
  }
  return float4(v_21, v_7);
}

float3x2 v_24(uint start_byte_offset) {
  uint4 v_25 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_26 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_25.zw) : (v_25.xy)));
  uint4 v_27 = arg_0_params[((8u + start_byte_offset) / 16u)];
  float2 v_28 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_27.zw) : (v_27.xy)));
  uint4 v_29 = arg_0_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_26, v_28, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_29.zw) : (v_29.xy))));
}

float3x3 v_30(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_31(uint start_byte_offset) {
  tint_GammaTransferParams v_32 = {asfloat(arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]), asfloat(arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]), arg_0_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_32;
}

float3x4 v_33(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_34(uint start_byte_offset) {
  uint v_35 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_36 = arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_37 = v_33((16u + start_byte_offset));
  tint_GammaTransferParams v_38 = v_31((64u + start_byte_offset));
  tint_GammaTransferParams v_39 = v_31((96u + start_byte_offset));
  float3x3 v_40 = v_30((128u + start_byte_offset));
  float3x2 v_41 = v_24((176u + start_byte_offset));
  float3x2 v_42 = v_24((200u + start_byte_offset));
  uint4 v_43 = arg_0_params[((224u + start_byte_offset) / 16u)];
  float2 v_44 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_43.zw) : (v_43.xy)));
  uint4 v_45 = arg_0_params[((232u + start_byte_offset) / 16u)];
  float2 v_46 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_45.zw) : (v_45.xy)));
  uint4 v_47 = arg_0_params[((240u + start_byte_offset) / 16u)];
  float2 v_48 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_47.zw) : (v_47.xy)));
  uint4 v_49 = arg_0_params[((248u + start_byte_offset) / 16u)];
  float2 v_50 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_49.zw) : (v_49.xy)));
  uint4 v_51 = arg_0_params[((256u + start_byte_offset) / 16u)];
  uint2 v_52 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_51.zw) : (v_51.xy));
  uint4 v_53 = arg_0_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_54 = {v_35, v_36, v_37, v_38, v_39, v_40, v_41, v_42, v_44, v_46, v_48, v_50, v_52, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_53.zw) : (v_53.xy)))};
  return v_54;
}

float4 textureLoad_8acf41() {
  int2 arg_1 = (int(1)).xx;
  tint_ExternalTextureParams v_55 = v_34(0u);
  float4 res = tint_TextureLoadExternal(arg_0_plane0, arg_0_plane1, v_55, uint2(arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_8acf41()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_8acf41()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_8acf41();
  VertexOutput v_56 = tint_symbol;
  return v_56;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_57 = vertex_main_inner();
  vertex_main_outputs v_58 = {v_57.prevent_dce, v_57.pos};
  return v_58;
}

