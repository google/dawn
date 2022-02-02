struct frexp_result {
  float sig;
  int exp;
};
frexp_result tint_frexp(float param_0) {
  float exp;
  float sig = frexp(param_0, exp);
  frexp_result result = {sig, int(exp)};
  return result;
}

void frexp_eabd40() {
  frexp_result res = tint_frexp(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_eabd40();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_eabd40();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_eabd40();
  return;
}
