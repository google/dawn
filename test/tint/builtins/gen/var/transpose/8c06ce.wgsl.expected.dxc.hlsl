void transpose_8c06ce() {
  matrix<float16_t, 3, 4> arg_0 = matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
  matrix<float16_t, 4, 3> res = transpose(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_8c06ce();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_8c06ce();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_8c06ce();
  return;
}
