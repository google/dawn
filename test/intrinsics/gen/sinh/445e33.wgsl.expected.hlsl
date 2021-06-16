void sinh_445e33() {
  float4 res = sinh(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  sinh_445e33();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  sinh_445e33();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sinh_445e33();
  return;
}
