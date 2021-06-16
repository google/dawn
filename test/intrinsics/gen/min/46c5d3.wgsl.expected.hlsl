void min_46c5d3() {
  uint res = min(1u, 1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  min_46c5d3();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  min_46c5d3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_46c5d3();
  return;
}
