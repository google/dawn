void cos_c5c28e() {
  float res = cos(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  cos_c5c28e();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  cos_c5c28e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cos_c5c28e();
  return;
}
