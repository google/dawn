struct ExternalTextureParams {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
};

Texture2D<float4> ext_tex_plane_1 : register(t1, space1);
cbuffer cbuffer_ext_tex_params : register(b2, space1) {
  uint4 ext_tex_params[2];
};
Texture2D<float4> arg_0 : register(t0, space1);

float4 textureLoadExternal(Texture2D<float4> plane0, Texture2D<float4> plane1, int2 coord, ExternalTextureParams params) {
  if ((params.numPlanes == 1u)) {
    return plane0.Load(int3(coord, 0));
  }
  const float y = (plane0.Load(int3(coord, 0)).r - 0.0625f);
  const float2 uv = (plane1.Load(int3(coord, 0)).rg - 0.5f);
  const float u = uv.x;
  const float v = uv.y;
  const float r = ((1.164000034f * y) + (params.vr * v));
  const float g = (((1.164000034f * y) - (params.ug * u)) - (params.vg * v));
  const float b = ((1.164000034f * y) + (params.ub * u));
  return float4(r, g, b, 1.0f);
}

float4 textureLoad2d(Texture2D<float4> tint_symbol, Texture2D<float4> ext_tex_plane_1_1, ExternalTextureParams ext_tex_params_1, int2 coords) {
  return textureLoadExternal(tint_symbol, ext_tex_plane_1_1, coords, ext_tex_params_1);
}

ExternalTextureParams tint_symbol_2(uint4 buffer[2], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  const uint scalar_offset_3 = ((offset + 12u)) / 4;
  const uint scalar_offset_4 = ((offset + 16u)) / 4;
  const ExternalTextureParams tint_symbol_5 = {buffer[scalar_offset / 4][scalar_offset % 4], asfloat(buffer[scalar_offset_1 / 4][scalar_offset_1 % 4]), asfloat(buffer[scalar_offset_2 / 4][scalar_offset_2 % 4]), asfloat(buffer[scalar_offset_3 / 4][scalar_offset_3 % 4]), asfloat(buffer[scalar_offset_4 / 4][scalar_offset_4 % 4])};
  return tint_symbol_5;
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
