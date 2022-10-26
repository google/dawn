Texture2DArray<float4> arg_0 : register(t0, space1);

void textureLoad_f348d9() {
  uint2 arg_1 = (0u).xx;
  uint arg_2 = 1u;
  int arg_3 = 1;
  float4 res = arg_0.Load(uint4(uint3(arg_1, arg_2), uint(arg_3)));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureLoad_f348d9();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureLoad_f348d9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_f348d9();
  return;
}
