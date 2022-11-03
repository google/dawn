void extractBits_a99a8d() {
  int2 res = (0).xx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  extractBits_a99a8d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  extractBits_a99a8d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  extractBits_a99a8d();
  return;
}
