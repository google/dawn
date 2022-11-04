RWTexture2D<float4> arg_0 : register(u0, space1);

void textureStore_1e20f2() {
  uint2 arg_1 = (1u).xx;
  float4 arg_2 = (1.0f).xxxx;
  arg_0[arg_1] = arg_2;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureStore_1e20f2();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureStore_1e20f2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_1e20f2();
  return;
}
