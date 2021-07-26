Texture2DMS<float4> arg_0 : register(t0, space1);

void ignore_2a6ac2() {
  arg_0;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  ignore_2a6ac2();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  ignore_2a6ac2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ignore_2a6ac2();
  return;
}
