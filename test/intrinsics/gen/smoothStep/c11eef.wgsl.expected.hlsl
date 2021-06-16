void smoothStep_c11eef() {
  float2 res = smoothstep(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  smoothStep_c11eef();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  smoothStep_c11eef();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothStep_c11eef();
  return;
}
