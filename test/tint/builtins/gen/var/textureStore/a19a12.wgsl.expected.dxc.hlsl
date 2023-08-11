RWTexture3D<uint4> arg_0 : register(u0, space1);

void textureStore_a19a12() {
  int3 arg_1 = (1).xxx;
  uint4 arg_2 = (1u).xxxx;
  arg_0[arg_1] = arg_2;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureStore_a19a12();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureStore_a19a12();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_a19a12();
  return;
}
