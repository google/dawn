void ldexp_db8b49() {
  float res = ldexp(1.0f, 1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  ldexp_db8b49();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  ldexp_db8b49();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_db8b49();
  return;
}
