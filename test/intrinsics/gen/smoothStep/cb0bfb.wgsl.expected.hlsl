void smoothStep_cb0bfb() {
  float res = smoothstep(1.0f, 1.0f, 1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  smoothStep_cb0bfb();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  smoothStep_cb0bfb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothStep_cb0bfb();
  return;
}
