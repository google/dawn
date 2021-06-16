void dot_ba4246() {
  float res = dot(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  dot_ba4246();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  dot_ba4246();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  dot_ba4246();
  return;
}
