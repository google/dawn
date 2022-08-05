void floor_b6e09c() {
  float16_t res = floor(float16_t(0.0h));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  floor_b6e09c();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  floor_b6e09c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  floor_b6e09c();
  return;
}
