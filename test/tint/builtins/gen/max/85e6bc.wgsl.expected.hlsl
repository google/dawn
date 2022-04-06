void max_85e6bc() {
  int4 res = max(int4(0, 0, 0, 0), int4(0, 0, 0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  max_85e6bc();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  max_85e6bc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_85e6bc();
  return;
}
