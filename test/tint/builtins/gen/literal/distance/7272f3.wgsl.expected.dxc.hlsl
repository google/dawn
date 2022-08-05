void distance_7272f3() {
  float16_t res = distance((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  distance_7272f3();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  distance_7272f3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  distance_7272f3();
  return;
}
