void ldexp_7bc2fd() {
  float2 res = ldexp(float2(0.0f, 0.0f), uint2(0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  ldexp_7bc2fd();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  ldexp_7bc2fd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_7bc2fd();
  return;
}
