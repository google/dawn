void ldexp_f54ff2() {
  float res = ldexp(1.0f, 1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  ldexp_f54ff2();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  ldexp_f54ff2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_f54ff2();
  return;
}
