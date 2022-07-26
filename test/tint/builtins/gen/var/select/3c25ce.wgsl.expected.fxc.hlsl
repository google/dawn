void select_3c25ce() {
  bool3 arg_0 = (true).xxx;
  bool3 arg_1 = (true).xxx;
  bool arg_2 = true;
  bool3 res = (arg_2 ? arg_1 : arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  select_3c25ce();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  select_3c25ce();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_3c25ce();
  return;
}
