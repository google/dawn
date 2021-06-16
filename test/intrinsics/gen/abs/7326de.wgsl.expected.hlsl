void abs_7326de() {
  uint3 res = abs(uint3(0u, 0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  abs_7326de();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  abs_7326de();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_7326de();
  return;
}
