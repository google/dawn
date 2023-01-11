int2 tint_clamp(int2 e, int2 low, int2 high) {
  return min(max(e, low), high);
}

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

Texture2D<float4> ext_tex_plane_1 : register(t2, space0);
cbuffer cbuffer_ext_tex_params : register(b3, space0) {
  uint4 ext_tex_params[13];
};
Texture2D<float4> t : register(t0, space0);
RWTexture2D<float4> outImage : register(u1, space0);

float3 gammaCorrection(float3 v, GammaTransferParams params) {
  const bool3 cond = (abs(v) < float3((params.D).xxx));
  const float3 t_1 = (float3(sign(v)) * ((params.C * abs(v)) + params.F));
  const float3 f = (float3(sign(v)) * (pow(((params.A * abs(v)) + params.B), float3((params.G).xxx)) + params.E));
  return (cond ? t_1 : f);
}

float4 textureLoadExternal(Texture2D<float4> plane0, Texture2D<float4> plane1, int2 coord, ExternalTextureParams params) {
  float3 color = float3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = plane0.Load(int3(coord, 0)).rgb;
  } else {
    color = mul(params.yuvToRgbConversionMatrix, float4(plane0.Load(int3(coord, 0)).r, plane1.Load(int3(coord, 0)).rg, 1.0f));
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = mul(color, params.gamutConversionMatrix);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return float4(color, 1.0f);
}

float3x4 tint_symbol_6(uint4 buffer[13], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]));
}

GammaTransferParams tint_symbol_8(uint4 buffer[13], uint offset) {
  const uint scalar_offset_3 = ((offset + 0u)) / 4;
  const uint scalar_offset_4 = ((offset + 4u)) / 4;
  const uint scalar_offset_5 = ((offset + 8u)) / 4;
  const uint scalar_offset_6 = ((offset + 12u)) / 4;
  const uint scalar_offset_7 = ((offset + 16u)) / 4;
  const uint scalar_offset_8 = ((offset + 20u)) / 4;
  const uint scalar_offset_9 = ((offset + 24u)) / 4;
  const uint scalar_offset_10 = ((offset + 28u)) / 4;
  const GammaTransferParams tint_symbol_14 = {asfloat(buffer[scalar_offset_3 / 4][scalar_offset_3 % 4]), asfloat(buffer[scalar_offset_4 / 4][scalar_offset_4 % 4]), asfloat(buffer[scalar_offset_5 / 4][scalar_offset_5 % 4]), asfloat(buffer[scalar_offset_6 / 4][scalar_offset_6 % 4]), asfloat(buffer[scalar_offset_7 / 4][scalar_offset_7 % 4]), asfloat(buffer[scalar_offset_8 / 4][scalar_offset_8 % 4]), asfloat(buffer[scalar_offset_9 / 4][scalar_offset_9 % 4]), buffer[scalar_offset_10 / 4][scalar_offset_10 % 4]};
  return tint_symbol_14;
}

float3x3 tint_symbol_10(uint4 buffer[13], uint offset) {
  const uint scalar_offset_11 = ((offset + 0u)) / 4;
  const uint scalar_offset_12 = ((offset + 16u)) / 4;
  const uint scalar_offset_13 = ((offset + 32u)) / 4;
  return float3x3(asfloat(buffer[scalar_offset_11 / 4].xyz), asfloat(buffer[scalar_offset_12 / 4].xyz), asfloat(buffer[scalar_offset_13 / 4].xyz));
}

float3x2 tint_symbol_12(uint4 buffer[13], uint offset) {
  const uint scalar_offset_14 = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset_14 / 4];
  const uint scalar_offset_15 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_15 / 4];
  const uint scalar_offset_16 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = buffer[scalar_offset_16 / 4];
  return float3x2(asfloat(((scalar_offset_14 & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_15 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_16 & 2) ? ubo_load_2.zw : ubo_load_2.xy)));
}

ExternalTextureParams tint_symbol_4(uint4 buffer[13], uint offset) {
  const uint scalar_offset_17 = ((offset + 0u)) / 4;
  const uint scalar_offset_18 = ((offset + 4u)) / 4;
  const ExternalTextureParams tint_symbol_15 = {buffer[scalar_offset_17 / 4][scalar_offset_17 % 4], buffer[scalar_offset_18 / 4][scalar_offset_18 % 4], tint_symbol_6(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 64u)), tint_symbol_8(buffer, (offset + 96u)), tint_symbol_10(buffer, (offset + 128u)), tint_symbol_12(buffer, (offset + 176u))};
  return tint_symbol_15;
}

[numthreads(1, 1, 1)]
void main() {
  int2 tint_tmp;
  t.GetDimensions(tint_tmp.x, tint_tmp.y);
  const int2 tint_symbol = tint_clamp((10).xx, (0).xx, int2((uint2(tint_tmp) - (1u).xx)));
  float4 red = textureLoadExternal(t, ext_tex_plane_1, tint_symbol, tint_symbol_4(ext_tex_params, 0u));
  int2 tint_tmp_1;
  outImage.GetDimensions(tint_tmp_1.x, tint_tmp_1.y);
  const int2 tint_symbol_1 = tint_clamp((0).xx, (0).xx, int2((uint2(tint_tmp_1) - (1u).xx)));
  outImage[tint_symbol_1] = red;
  int2 tint_tmp_2;
  t.GetDimensions(tint_tmp_2.x, tint_tmp_2.y);
  const int2 tint_symbol_2 = tint_clamp(int2(70, 118), (0).xx, int2((uint2(tint_tmp_2) - (1u).xx)));
  float4 green = textureLoadExternal(t, ext_tex_plane_1, tint_symbol_2, tint_symbol_4(ext_tex_params, 0u));
  int2 tint_tmp_3;
  outImage.GetDimensions(tint_tmp_3.x, tint_tmp_3.y);
  const int2 tint_symbol_3 = tint_clamp(int2(1, 0), (0).xx, int2((uint2(tint_tmp_3) - (1u).xx)));
  outImage[tint_symbol_3] = green;
  return;
}
