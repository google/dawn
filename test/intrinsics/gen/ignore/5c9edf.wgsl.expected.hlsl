Texture2D<float4> arg_0 : register(t0, space1);

void ignore_5c9edf() {
  arg_0;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  ignore_5c9edf();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  ignore_5c9edf();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ignore_5c9edf();
  return;
}
