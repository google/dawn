void acos_8e2acf() {
  float4 res = acos(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  acos_8e2acf();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  acos_8e2acf();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_8e2acf();
  return;
}
