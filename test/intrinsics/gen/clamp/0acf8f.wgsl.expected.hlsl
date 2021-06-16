void clamp_0acf8f() {
  float2 res = clamp(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  clamp_0acf8f();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  clamp_0acf8f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_0acf8f();
  return;
}
