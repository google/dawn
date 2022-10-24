Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_faa6d7() {
  float4 res = arg_0.SampleLevel(arg_1, float3(0.0f, 0.0f, float(1u)), 1.0f, (0).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureSampleLevel_faa6d7();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureSampleLevel_faa6d7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_faa6d7();
  return;
}
