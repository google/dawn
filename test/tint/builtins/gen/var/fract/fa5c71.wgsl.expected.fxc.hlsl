void fract_fa5c71() {
  float arg_0 = 1.25f;
  float res = frac(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  fract_fa5c71();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  fract_fa5c71();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_fa5c71();
  return;
}
