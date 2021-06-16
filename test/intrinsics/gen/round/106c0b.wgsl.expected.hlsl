void round_106c0b() {
  float4 res = round(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  round_106c0b();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  round_106c0b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  round_106c0b();
  return;
}
