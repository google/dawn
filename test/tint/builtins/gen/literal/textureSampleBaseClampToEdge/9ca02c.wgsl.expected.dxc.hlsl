float4 tint_textureSampleBaseClampToEdge(Texture2D<float4> t, SamplerState s, float2 coord) {
  uint3 tint_tmp;
  t.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  const float2 dims = float2(tint_tmp.xy);
  const float2 half_texel = ((0.5f).xx / dims);
  const float2 clamped = clamp(coord, half_texel, (1.0f - half_texel));
  return t.SampleLevel(s, clamped, 0.0f);
}

Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSampleBaseClampToEdge_9ca02c() {
  float4 res = tint_textureSampleBaseClampToEdge(arg_0, arg_1, (1.0f).xx);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureSampleBaseClampToEdge_9ca02c();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureSampleBaseClampToEdge_9ca02c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleBaseClampToEdge_9ca02c();
  return;
}
