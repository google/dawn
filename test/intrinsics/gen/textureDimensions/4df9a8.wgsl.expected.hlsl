RWTexture1D<uint4> arg_0 : register(u0, space1);

void textureDimensions_4df9a8() {
  int tint_tmp;
  arg_0.GetDimensions(tint_tmp);
  int res = tint_tmp;
}

void vertex_main() {
  textureDimensions_4df9a8();
  return;
}

void fragment_main() {
  textureDimensions_4df9a8();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_4df9a8();
  return;
}

