TextureCube<int4> arg_1 : register(t1, space1);
SamplerState arg_2 : register(s2, space1);

void textureGather_f7995a() {
  int4 res = arg_1.GatherGreen(arg_2, float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureGather_f7995a();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureGather_f7995a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureGather_f7995a();
  return;
}
