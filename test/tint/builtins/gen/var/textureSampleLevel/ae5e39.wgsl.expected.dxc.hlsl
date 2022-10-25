TextureCubeArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_ae5e39() {
  float3 arg_2 = (0.0f).xxx;
  int arg_3 = 1;
  int arg_4 = 1;
  float res = arg_0.SampleLevel(arg_1, float4(arg_2, float(arg_3)), arg_4).x;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureSampleLevel_ae5e39();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureSampleLevel_ae5e39();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_ae5e39();
  return;
}
