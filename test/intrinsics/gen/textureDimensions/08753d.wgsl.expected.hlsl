RWTexture1D<int4> arg_0 : register(u0, space1);

void textureDimensions_08753d() {
  int tint_tmp;
  arg_0.GetDimensions(tint_tmp);
  int res = tint_tmp;
}

void vertex_main() {
  textureDimensions_08753d();
  return;
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

