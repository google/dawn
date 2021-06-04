struct tint_symbol {
  float4 value : SV_Position;
};

void reflect_feae90() {
  float res = reflect(1.0f, 1.0f);
}

tint_symbol vertex_main() {
  reflect_feae90();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  reflect_feae90();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reflect_feae90();
  return;
}

