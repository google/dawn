void ceil_b74c16() {
  float4 res = ceil(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  ceil_b74c16();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  ceil_b74c16();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ceil_b74c16();
  return;
}
