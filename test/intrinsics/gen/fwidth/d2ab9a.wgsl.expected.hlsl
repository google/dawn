void fwidth_d2ab9a() {
  float4 res = fwidth(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  fwidth_d2ab9a();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  fwidth_d2ab9a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidth_d2ab9a();
  return;
}
