struct ExternalTextureParams {
  uint numPlanes;
  float3x4 yuvToRgbConversionMatrix;
};

Texture2D<float4> ext_tex_plane_1 : register(t1, space1);
cbuffer cbuffer_ext_tex_params : register(b2, space1) {
  uint4 ext_tex_params[4];
};
Texture2D<float4> arg_0 : register(t0, space1);

float4 textureLoadExternal(Texture2D<float4> plane0, Texture2D<float4> plane1, int2 coord, ExternalTextureParams params) {
  float3 color = float3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = plane0.Load(int3(coord, 0)).rgb;
  } else {
    color = mul(params.yuvToRgbConversionMatrix, float4(plane0.Load(int3(coord, 0)).r, plane1.Load(int3(coord, 0)).rg, 1.0f));
  }
  return float4(color, 1.0f);
}

float3x4 tint_symbol_3(uint4 buffer[4], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]));
}

ExternalTextureParams tint_symbol_1(uint4 buffer[4], uint offset) {
  const uint scalar_offset_3 = ((offset + 0u)) / 4;
  const ExternalTextureParams tint_symbol_5 = {buffer[scalar_offset_3 / 4][scalar_offset_3 % 4], tint_symbol_3(buffer, (offset + 16u))};
  return tint_symbol_5;
}

void textureLoad_8acf41() {
  float4 res = textureLoadExternal(arg_0, ext_tex_plane_1, int2(0, 0), tint_symbol_1(ext_tex_params, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureLoad_8acf41();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureLoad_8acf41();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_8acf41();
  return;
}
