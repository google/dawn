RWTexture1D<int4> arg_0 : register(u0, space1);

void textureDimensions_cc968c() {
  int tint_tmp;
  arg_0.GetDimensions(tint_tmp);
  int res = tint_tmp;
}

void vertex_main() {
  textureDimensions_cc968c();
  return;
}

void fragment_main() {
  textureDimensions_cc968c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_cc968c();
  return;
}

