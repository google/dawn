static float4 arg_1 = float4(0.0f, 0.0f, 0.0f, 0.0f);

void modf_3d00e2() {
  float4 res = modf(float4(0.0f, 0.0f, 0.0f, 0.0f), arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  modf_3d00e2();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  modf_3d00e2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_3d00e2();
  return;
}
