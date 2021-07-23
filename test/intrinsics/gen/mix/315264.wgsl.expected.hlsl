void mix_315264() {
  float3 res = lerp(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), 1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  mix_315264();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  mix_315264();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  mix_315264();
  return;
}
