Texture1D<uint4> arg_0 : register(t0, space1);

void textureDimensions_441392() {
  int tint_tmp;
  arg_0.GetDimensions(tint_tmp);
  int res = tint_tmp;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  textureDimensions_441392();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureDimensions_441392();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_441392();
  return;
}
