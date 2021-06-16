void abs_002533() {
  float4 res = abs(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  abs_002533();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  abs_002533();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_002533();
  return;
}
