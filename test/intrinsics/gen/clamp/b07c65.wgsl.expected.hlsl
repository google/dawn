struct tint_symbol {
  float4 value : SV_Position;
};

void clamp_b07c65() {
  int res = clamp(1, 1, 1);
}

tint_symbol vertex_main() {
  clamp_b07c65();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  clamp_b07c65();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_b07c65();
  return;
}

