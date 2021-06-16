void all_986c7b() {
  bool res = all(bool4(false, false, false, false));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  all_986c7b();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  all_986c7b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  all_986c7b();
  return;
}
