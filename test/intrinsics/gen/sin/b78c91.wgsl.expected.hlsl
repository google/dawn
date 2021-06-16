void sin_b78c91() {
  float res = sin(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  sin_b78c91();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  sin_b78c91();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sin_b78c91();
  return;
}
