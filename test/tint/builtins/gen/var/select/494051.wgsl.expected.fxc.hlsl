void select_494051() {
  bool arg_2 = true;
  int res = (arg_2 ? 1 : 1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  select_494051();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  select_494051();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_494051();
  return;
}
