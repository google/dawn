void frexp_19ab15() {
  int4 arg_1 = int4(0, 0, 0, 0);
  float4 tint_tmp;
  float4 tint_tmp_1 = frexp(float4(0.0f, 0.0f, 0.0f, 0.0f), tint_tmp);
  arg_1 = int4(tint_tmp);
  float4 res = tint_tmp_1;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  frexp_19ab15();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_19ab15();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_19ab15();
  return;
}
