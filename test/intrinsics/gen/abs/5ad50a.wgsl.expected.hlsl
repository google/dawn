void abs_5ad50a() {
  int3 res = abs(int3(0, 0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  abs_5ad50a();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  abs_5ad50a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_5ad50a();
  return;
}
