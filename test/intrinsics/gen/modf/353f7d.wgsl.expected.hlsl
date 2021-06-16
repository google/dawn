void modf_353f7d() {
  float arg_1 = 0.0f;
  float res = modf(1.0f, arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  modf_353f7d();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  modf_353f7d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_353f7d();
  return;
}
