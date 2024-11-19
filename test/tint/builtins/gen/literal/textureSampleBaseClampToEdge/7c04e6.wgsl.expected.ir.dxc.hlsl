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
Texture2D<float4> arg_0_plane1 : register(t2, space1);
cbuffer cbuffer_arg_0_params : register(b3, space1) {
  uint4 arg_0_params[17];
};
SamplerState arg_1 : register(s1, space1);
float3 tint_GammaCorrection(float3 v, tint_GammaTransferParams params) {
  float3 v_1 = float3((params.G).xxx);
  float3 v_2 = float3((params.D).xxx);
  float3 v_3 = float3(sign(v));
  return (((abs(v) < v_2)) ? ((v_3 * ((params.C * abs(v)) + params.F))) : ((v_3 * (pow(((params.A * abs(v)) + params.B), v_1) + params.E))));
}

float4 tint_TextureSampleExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, SamplerState tint_sampler, float2 coords) {
  float2 v_4 = mul(float3(coords, 1.0f), params.sampleTransform);
  float3 v_5 = (0.0f).xxx;
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    float4 v_7 = plane_0.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), float(0.0f));
    v_5 = v_7.xyz;
    v_6 = v_7.w;
  } else {
    float v_8 = plane_0.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), float(0.0f)).x;
    v_5 = mul(params.yuvToRgbConversionMatrix, float4(v_8, plane_1.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane1RectMin, params.samplePlane1RectMax), float(0.0f)).xy, 1.0f));
    v_6 = 1.0f;
  }
  float3 v_9 = v_5;
  float3 v_10 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_GammaTransferParams v_11 = params.gammaDecodeParams;
    tint_GammaTransferParams v_12 = params.gammaEncodeParams;
    v_10 = tint_GammaCorrection(mul(tint_GammaCorrection(v_9, v_11), params.gamutConversionMatrix), v_12);
  } else {
    v_10 = v_9;
  }
  return float4(v_10, v_6);
}

float3x2 v_13(uint start_byte_offset) {
  uint4 v_14 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_15 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_14.zw) : (v_14.xy)));
  uint4 v_16 = arg_0_params[((8u + start_byte_offset) / 16u)];
  float2 v_17 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_16.zw) : (v_16.xy)));
  uint4 v_18 = arg_0_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_15, v_17, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_18.zw) : (v_18.xy))));
}

float3x3 v_19(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_20(uint start_byte_offset) {
  tint_GammaTransferParams v_21 = {asfloat(arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]), asfloat(arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]), arg_0_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_21;
}

float3x4 v_22(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_23(uint start_byte_offset) {
  uint v_24 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_25 = arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_26 = v_22((16u + start_byte_offset));
  tint_GammaTransferParams v_27 = v_20((64u + start_byte_offset));
  tint_GammaTransferParams v_28 = v_20((96u + start_byte_offset));
  float3x3 v_29 = v_19((128u + start_byte_offset));
  float3x2 v_30 = v_13((176u + start_byte_offset));
  float3x2 v_31 = v_13((200u + start_byte_offset));
  uint4 v_32 = arg_0_params[((224u + start_byte_offset) / 16u)];
  float2 v_33 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_32.zw) : (v_32.xy)));
  uint4 v_34 = arg_0_params[((232u + start_byte_offset) / 16u)];
  float2 v_35 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_34.zw) : (v_34.xy)));
  uint4 v_36 = arg_0_params[((240u + start_byte_offset) / 16u)];
  float2 v_37 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_36.zw) : (v_36.xy)));
  uint4 v_38 = arg_0_params[((248u + start_byte_offset) / 16u)];
  float2 v_39 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_38.zw) : (v_38.xy)));
  uint4 v_40 = arg_0_params[((256u + start_byte_offset) / 16u)];
  uint2 v_41 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_40.zw) : (v_40.xy));
  uint4 v_42 = arg_0_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_43 = {v_24, v_25, v_26, v_27, v_28, v_29, v_30, v_31, v_33, v_35, v_37, v_39, v_41, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_42.zw) : (v_42.xy)))};
  return v_43;
}

float4 textureSampleBaseClampToEdge_7c04e6() {
  tint_ExternalTextureParams v_44 = v_23(0u);
  float4 res = tint_TextureSampleExternal(arg_0_plane0, arg_0_plane1, v_44, arg_1, (1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBaseClampToEdge_7c04e6()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBaseClampToEdge_7c04e6()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureSampleBaseClampToEdge_7c04e6();
  VertexOutput v_45 = tint_symbol;
  return v_45;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_46 = vertex_main_inner();
  vertex_main_outputs v_47 = {v_46.prevent_dce, v_46.pos};
  return v_47;
}

