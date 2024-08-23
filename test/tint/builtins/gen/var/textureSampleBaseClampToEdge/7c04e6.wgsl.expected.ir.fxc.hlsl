SKIP: FAILED

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
  float3 v_3 = abs(v);
  float3 v_4 = float3(sign(v));
  return (((v_3 < v_2)) ? ((v_4 * ((params.C * v_3) + params.F))) : ((v_4 * (pow(((params.A * v_3) + params.B), v_1) + params.E))));
}

float4 tint_TextureSampleExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, SamplerState sampler, float2 coords) {
  float2 v_5 = mul(float3(coords, 1.0f), params.sampleTransform);
  float2 v_6 = clamp(v_5, params.samplePlane0RectMin, params.samplePlane0RectMax);
  float3 v_7 = (0.0f).xxx;
  float v_8 = 0.0f;
  if ((params.numPlanes == 1u)) {
    float4 v_9 = plane_0.SampleLevel(sampler, v_6, float(0.0f));
    v_7 = v_9.xyz;
    v_8 = v_9[3u];
  } else {
    float v_10 = plane_0.SampleLevel(sampler, v_6, float(0.0f))[0u];
    float2 v_11 = clamp(v_5, params.samplePlane1RectMin, params.samplePlane1RectMax);
    v_7 = mul(params.yuvToRgbConversionMatrix, float4(v_10, plane_1.SampleLevel(sampler, v_11, float(0.0f)).xy, 1.0f));
    v_8 = 1.0f;
  }
  float3 v_12 = v_7;
  float3 v_13 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_GammaTransferParams v_14 = params.gammaDecodeParams;
    tint_GammaTransferParams v_15 = params.gammaEncodeParams;
    v_13 = tint_GammaCorrection(mul(tint_GammaCorrection(v_12, v_14), params.gamutConversionMatrix), v_15);
  } else {
    v_13 = v_12;
  }
  return float4(v_13, v_8);
}

float3x2 v_16(uint start_byte_offset) {
  uint4 v_17 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_18 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_17.zw) : (v_17.xy)));
  uint4 v_19 = arg_0_params[((8u + start_byte_offset) / 16u)];
  float2 v_20 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_19.zw) : (v_19.xy)));
  uint4 v_21 = arg_0_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_18, v_20, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_21.zw) : (v_21.xy))));
}

float3x3 v_22(uint start_byte_offset) {
  float3 v_23 = asfloat(arg_0_params[(start_byte_offset / 16u)].xyz);
  float3 v_24 = asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_23, v_24, asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_25(uint start_byte_offset) {
  float v_26 = asfloat(arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float v_27 = asfloat(arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  float v_28 = asfloat(arg_0_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  float v_29 = asfloat(arg_0_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]);
  float v_30 = asfloat(arg_0_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]);
  float v_31 = asfloat(arg_0_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]);
  float v_32 = asfloat(arg_0_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]);
  tint_GammaTransferParams v_33 = {v_26, v_27, v_28, v_29, v_30, v_31, v_32, arg_0_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_33;
}

float3x4 v_34(uint start_byte_offset) {
  float4 v_35 = asfloat(arg_0_params[(start_byte_offset / 16u)]);
  float4 v_36 = asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_35, v_36, asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_37(uint start_byte_offset) {
  uint v_38 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_39 = arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_40 = v_34((16u + start_byte_offset));
  tint_GammaTransferParams v_41 = v_25((64u + start_byte_offset));
  tint_GammaTransferParams v_42 = v_25((96u + start_byte_offset));
  float3x3 v_43 = v_22((128u + start_byte_offset));
  float3x2 v_44 = v_16((176u + start_byte_offset));
  float3x2 v_45 = v_16((200u + start_byte_offset));
  uint4 v_46 = arg_0_params[((224u + start_byte_offset) / 16u)];
  float2 v_47 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_46.zw) : (v_46.xy)));
  uint4 v_48 = arg_0_params[((232u + start_byte_offset) / 16u)];
  float2 v_49 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_48.zw) : (v_48.xy)));
  uint4 v_50 = arg_0_params[((240u + start_byte_offset) / 16u)];
  float2 v_51 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_50.zw) : (v_50.xy)));
  uint4 v_52 = arg_0_params[((248u + start_byte_offset) / 16u)];
  float2 v_53 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_52.zw) : (v_52.xy)));
  uint4 v_54 = arg_0_params[((256u + start_byte_offset) / 16u)];
  uint2 v_55 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_54.zw) : (v_54.xy));
  uint4 v_56 = arg_0_params[((264u + start_byte_offset) / 16u)];
  tint_GammaTransferParams v_57 = v_41;
  tint_GammaTransferParams v_58 = v_42;
  tint_ExternalTextureParams v_59 = {v_38, v_39, v_40, v_57, v_58, v_43, v_44, v_45, v_47, v_49, v_51, v_53, v_55, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_56.zw) : (v_56.xy)))};
  return v_59;
}

float4 textureSampleBaseClampToEdge_7c04e6() {
  float2 arg_2 = (1.0f).xx;
  Texture2D<float4> v_60 = arg_0_plane0;
  Texture2D<float4> v_61 = arg_0_plane1;
  tint_ExternalTextureParams v_62 = v_37(0u);
  tint_ExternalTextureParams v_63 = v_62;
  float4 res = tint_TextureSampleExternal(v_60, v_61, v_63, arg_1, arg_2);
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
  VertexOutput v_64 = tint_symbol;
  return v_64;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_65 = vertex_main_inner();
  VertexOutput v_66 = v_65;
  VertexOutput v_67 = v_65;
  vertex_main_outputs v_68 = {v_67.prevent_dce, v_66.pos};
  return v_68;
}

FXC validation failure:
<scrubbed_path>(55,137-143): error X3000: syntax error: unexpected token 'sampler'
<scrubbed_path>(56,27-32): error X3004: undeclared identifier 'coords'


tint executable returned error: exit status 1
