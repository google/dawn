void abs_4ad288() {
  int res = abs(1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  abs_4ad288();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  abs_4ad288();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_4ad288();
  return;
}
