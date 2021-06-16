Texture3D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_9bd37b() {
  float4 res = arg_0.SampleLevel(arg_1, float3(0.0f, 0.0f, 0.0f), 1.0f, int3(0, 0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  textureSampleLevel_9bd37b();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureSampleLevel_9bd37b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_9bd37b();
  return;
}
