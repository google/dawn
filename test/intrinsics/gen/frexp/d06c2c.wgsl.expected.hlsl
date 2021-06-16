void frexp_d06c2c() {
  int2 arg_1 = int2(0, 0);
  float2 tint_tmp;
  float2 tint_tmp_1 = frexp(float2(0.0f, 0.0f), tint_tmp);
  arg_1 = int2(tint_tmp);
  float2 res = tint_tmp_1;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  frexp_d06c2c();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_d06c2c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_d06c2c();
  return;
}
