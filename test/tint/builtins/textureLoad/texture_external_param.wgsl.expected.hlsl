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
  float3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  float3x3 gamutConversionMatrix;
};

Texture2D<float4> ext_tex_plane_1 : register(t1, space1);
cbuffer cbuffer_ext_tex_params : register(b2, space1) {
  uint4 ext_tex_params[11];
};
Texture2D<float4> arg_0 : register(t0, space1);

float3 gammaCorrection(float3 v, GammaTransferParams params) {
  const bool3 cond = (abs(v) < float3((params.D).xxx));
  const float3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  const float3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), float3((params.G).xxx)) + params.E));
  return (cond ? t : f);
}

float4 textureLoadExternal(Texture2D<float4> plane0, Texture2D<float4> plane1, int2 coord, ExternalTextureParams params) {
  float3 color = float3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = plane0.Load(int3(coord, 0)).rgb;
  } else {
    color = mul(params.yuvToRgbConversionMatrix, float4(plane0.Load(int3(coord, 0)).r, plane1.Load(int3(coord, 0)).rg, 1.0f));
  }
  color = gammaCorrection(color, params.gammaDecodeParams);
  color = mul(color, params.gamutConversionMatrix);
  color = gammaCorrection(color, params.gammaEncodeParams);
  return float4(color, 1.0f);
}

float4 textureLoad2d(Texture2D<float4> tint_symbol, Texture2D<float4> ext_tex_plane_1_1, ExternalTextureParams ext_tex_params_1, int2 coords) {
  return textureLoadExternal(tint_symbol, ext_tex_plane_1_1, coords, ext_tex_params_1);
}

float3x4 tint_symbol_4(uint4 buffer[11], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]));
}

GammaTransferParams tint_symbol_6(uint4 buffer[11], uint offset) {
  const uint scalar_offset_3 = ((offset + 0u)) / 4;
  const uint scalar_offset_4 = ((offset + 4u)) / 4;
  const uint scalar_offset_5 = ((offset + 8u)) / 4;
  const uint scalar_offset_6 = ((offset + 12u)) / 4;
  const uint scalar_offset_7 = ((offset + 16u)) / 4;
  const uint scalar_offset_8 = ((offset + 20u)) / 4;
  const uint scalar_offset_9 = ((offset + 24u)) / 4;
  const uint scalar_offset_10 = ((offset + 28u)) / 4;
  const GammaTransferParams tint_symbol_10 = {asfloat(buffer[scalar_offset_3 / 4][scalar_offset_3 % 4]), asfloat(buffer[scalar_offset_4 / 4][scalar_offset_4 % 4]), asfloat(buffer[scalar_offset_5 / 4][scalar_offset_5 % 4]), asfloat(buffer[scalar_offset_6 / 4][scalar_offset_6 % 4]), asfloat(buffer[scalar_offset_7 / 4][scalar_offset_7 % 4]), asfloat(buffer[scalar_offset_8 / 4][scalar_offset_8 % 4]), asfloat(buffer[scalar_offset_9 / 4][scalar_offset_9 % 4]), buffer[scalar_offset_10 / 4][scalar_offset_10 % 4]};
  return tint_symbol_10;
}

float3x3 tint_symbol_8(uint4 buffer[11], uint offset) {
  const uint scalar_offset_11 = ((offset + 0u)) / 4;
  const uint scalar_offset_12 = ((offset + 16u)) / 4;
  const uint scalar_offset_13 = ((offset + 32u)) / 4;
  return float3x3(asfloat(buffer[scalar_offset_11 / 4].xyz), asfloat(buffer[scalar_offset_12 / 4].xyz), asfloat(buffer[scalar_offset_13 / 4].xyz));
}

ExternalTextureParams tint_symbol_2(uint4 buffer[11], uint offset) {
  const uint scalar_offset_14 = ((offset + 0u)) / 4;
  const ExternalTextureParams tint_symbol_11 = {buffer[scalar_offset_14 / 4][scalar_offset_14 % 4], tint_symbol_4(buffer, (offset + 16u)), tint_symbol_6(buffer, (offset + 64u)), tint_symbol_6(buffer, (offset + 96u)), tint_symbol_8(buffer, (offset + 128u))};
  return tint_symbol_11;
}

void doTextureLoad() {
  float4 res = textureLoad2d(arg_0, ext_tex_plane_1, tint_symbol_2(ext_tex_params, 0u), int2(0, 0));
}

struct tint_symbol_1 {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  doTextureLoad();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol_1 vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  doTextureLoad();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  doTextureLoad();
  return;
}
