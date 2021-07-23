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

void frexp_12f1da() {
  frexp_result res = tint_frexp(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  frexp_12f1da();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_12f1da();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_12f1da();
  return;
}
