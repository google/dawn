void trunc_e183aa() {
  float4 res = trunc(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  trunc_e183aa();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  trunc_e183aa();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  trunc_e183aa();
  return;
}
