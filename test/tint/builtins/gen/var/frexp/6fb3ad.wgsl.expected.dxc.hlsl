struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};
void frexp_6fb3ad() {
  frexp_result_vec2_f32 res = {(0.5f).xx, (1).xx};
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_6fb3ad();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_6fb3ad();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_6fb3ad();
  return;
}
