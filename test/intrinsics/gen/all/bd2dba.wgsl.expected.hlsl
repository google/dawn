void all_bd2dba() {
  bool res = all(bool3(false, false, false));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  all_bd2dba();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  all_bd2dba();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  all_bd2dba();
  return;
}
