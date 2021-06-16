void select_a2860e() {
  int4 res = (bool4(false, false, false, false) ? int4(0, 0, 0, 0) : int4(0, 0, 0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_a2860e();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_a2860e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_a2860e();
  return;
}
