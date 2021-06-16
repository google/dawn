RWTexture1D<int4> arg_0 : register(u0, space1);

void textureDimensions_08753d() {
  int tint_tmp;
  arg_0.GetDimensions(tint_tmp);
  int res = tint_tmp;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  textureDimensions_08753d();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureDimensions_08753d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_08753d();
  return;
}
