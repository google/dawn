Texture2DMS<uint4> arg_0 : register(t0, space1);

void textureLoad_cad5f2() {
  uint4 res = arg_0.Load((1).xx, 1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureLoad_cad5f2();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureLoad_cad5f2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_cad5f2();
  return;
}
