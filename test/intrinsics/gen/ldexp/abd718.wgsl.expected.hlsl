void ldexp_abd718() {
  float2 res = ldexp(float2(0.0f, 0.0f), int2(0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  ldexp_abd718();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  ldexp_abd718();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_abd718();
  return;
}
