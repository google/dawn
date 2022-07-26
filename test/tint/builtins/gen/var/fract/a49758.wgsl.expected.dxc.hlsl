void fract_a49758() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = frac(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  fract_a49758();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  fract_a49758();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_a49758();
  return;
}
