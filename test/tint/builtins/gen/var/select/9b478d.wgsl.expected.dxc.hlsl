void select_9b478d() {
  bool arg_2 = true;
  int res = (arg_2 ? 1 : 1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  select_9b478d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  select_9b478d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_9b478d();
  return;
}
