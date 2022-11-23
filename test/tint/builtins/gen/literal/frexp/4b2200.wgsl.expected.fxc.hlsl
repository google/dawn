struct frexp_result_f32 {
  float fract;
  int exp;
};
void frexp_4b2200() {
  frexp_result_f32 res = {0.5f, 1};
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_4b2200();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_4b2200();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_4b2200();
  return;
}
