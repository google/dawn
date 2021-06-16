void modf_d1d6f6() {
  float4 arg_1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 res = modf(float4(0.0f, 0.0f, 0.0f, 0.0f), arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  modf_d1d6f6();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  modf_d1d6f6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_d1d6f6();
  return;
}
