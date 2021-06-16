void floor_3bccc4() {
  float4 res = floor(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  floor_3bccc4();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  floor_3bccc4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  floor_3bccc4();
  return;
}
