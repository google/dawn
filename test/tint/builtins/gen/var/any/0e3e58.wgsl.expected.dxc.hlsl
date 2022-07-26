void any_0e3e58() {
  bool2 arg_0 = (true).xx;
  bool res = any(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  any_0e3e58();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  any_0e3e58();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  any_0e3e58();
  return;
}
