void asin_a6d73a() {
  float res = 0.5f;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  asin_a6d73a();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  asin_a6d73a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asin_a6d73a();
  return;
}
