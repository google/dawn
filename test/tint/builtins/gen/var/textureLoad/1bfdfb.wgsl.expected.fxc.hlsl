struct GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};
struct ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  float3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  float3x3 gamutConversionMatrix;
  float3x2 coordTransformationMatrix;
};

Texture2D<float4> ext_tex_plane_1 : register(t1, space1);
cbuffer cbuffer_ext_tex_params : register(b2, space1) {
  uint4 ext_tex_params[13];
};
Texture2D<float4> arg_0 : register(t0, space1);

float3 gammaCorrection(float3 v, GammaTransferParams params) {
  const bool3 cond = (abs(v) < float3((params.D).xxx));
  const float3 t = (float3(sign(v)) * ((params.C * abs(v)) + params.F));
  const float3 f = (float3(sign(v)) * (pow(((params.A * abs(v)) + params.B), float3((params.G).xxx)) + params.E));
  return (cond ? t : f);
}

float4 textureLoadExternal(Texture2D<float4> plane0, Texture2D<float4> plane1, uint2 coord, ExternalTextureParams params) {
  const uint2 coord1 = (coord >> (1u).xx);
  float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = plane0.Load(uint3(coord, uint(0))).rgba;
  } else {
    color = float4(mul(params.yuvToRgbConversionMatrix, float4(plane0.Load(uint3(coord, uint(0))).r, plane1.Load(uint3(coord1, uint(0))).rg, 1.0f)), 1.0f);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = float4(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = float4(mul(color.rgb, params.gamutConversionMatrix), color.a);
    color = float4(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

float3x4 ext_tex_params_load_2(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(ext_tex_params[scalar_offset / 4]), asfloat(ext_tex_params[scalar_offset_1 / 4]), asfloat(ext_tex_params[scalar_offset_2 / 4]));
}

GammaTransferParams ext_tex_params_load_4(uint offset) {
  const uint scalar_offset_3 = ((offset + 0u)) / 4;
  const uint scalar_offset_4 = ((offset + 4u)) / 4;
  const uint scalar_offset_5 = ((offset + 8u)) / 4;
  const uint scalar_offset_6 = ((offset + 12u)) / 4;
  const uint scalar_offset_7 = ((offset + 16u)) / 4;
  const uint scalar_offset_8 = ((offset + 20u)) / 4;
  const uint scalar_offset_9 = ((offset + 24u)) / 4;
  const uint scalar_offset_10 = ((offset + 28u)) / 4;
  const GammaTransferParams tint_symbol_1 = {asfloat(ext_tex_params[scalar_offset_3 / 4][scalar_offset_3 % 4]), asfloat(ext_tex_params[scalar_offset_4 / 4][scalar_offset_4 % 4]), asfloat(ext_tex_params[scalar_offset_5 / 4][scalar_offset_5 % 4]), asfloat(ext_tex_params[scalar_offset_6 / 4][scalar_offset_6 % 4]), asfloat(ext_tex_params[scalar_offset_7 / 4][scalar_offset_7 % 4]), asfloat(ext_tex_params[scalar_offset_8 / 4][scalar_offset_8 % 4]), asfloat(ext_tex_params[scalar_offset_9 / 4][scalar_offset_9 % 4]), ext_tex_params[scalar_offset_10 / 4][scalar_offset_10 % 4]};
  return tint_symbol_1;
}

float3x3 ext_tex_params_load_6(uint offset) {
  const uint scalar_offset_11 = ((offset + 0u)) / 4;
  const uint scalar_offset_12 = ((offset + 16u)) / 4;
  const uint scalar_offset_13 = ((offset + 32u)) / 4;
  return float3x3(asfloat(ext_tex_params[scalar_offset_11 / 4].xyz), asfloat(ext_tex_params[scalar_offset_12 / 4].xyz), asfloat(ext_tex_params[scalar_offset_13 / 4].xyz));
}

float3x2 ext_tex_params_load_8(uint offset) {
  const uint scalar_offset_14 = ((offset + 0u)) / 4;
  uint4 ubo_load = ext_tex_params[scalar_offset_14 / 4];
  const uint scalar_offset_15 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = ext_tex_params[scalar_offset_15 / 4];
  const uint scalar_offset_16 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = ext_tex_params[scalar_offset_16 / 4];
  return float3x2(asfloat(((scalar_offset_14 & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_15 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_16 & 2) ? ubo_load_2.zw : ubo_load_2.xy)));
}

ExternalTextureParams ext_tex_params_load(uint offset) {
  const uint scalar_offset_17 = ((offset + 0u)) / 4;
  const uint scalar_offset_18 = ((offset + 4u)) / 4;
  ExternalTextureParams tint_symbol_2 = {ext_tex_params[scalar_offset_17 / 4][scalar_offset_17 % 4], ext_tex_params[scalar_offset_18 / 4][scalar_offset_18 % 4], ext_tex_params_load_2((offset + 16u)), ext_tex_params_load_4((offset + 64u)), ext_tex_params_load_4((offset + 96u)), ext_tex_params_load_6((offset + 128u)), ext_tex_params_load_8((offset + 176u))};
  return tint_symbol_2;
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureLoad_1bfdfb() {
  uint2 arg_1 = (1u).xx;
  float4 res = textureLoadExternal(arg_0, ext_tex_plane_1, arg_1, ext_tex_params_load(0u));
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureLoad_1bfdfb();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureLoad_1bfdfb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_1bfdfb();
  return;
}
