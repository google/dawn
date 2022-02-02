void max_e8192f() {
  int2 res = max(int2(0, 0), int2(0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  max_e8192f();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  max_e8192f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_e8192f();
  return;
}
