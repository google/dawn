Texture1D<uint4> arg_0 : register(t0, space1);

void textureDimensions_70e26a() {
  int tint_tmp;
  arg_0.GetDimensions(tint_tmp);
  int res = tint_tmp;
}

void vertex_main() {
  textureDimensions_70e26a();
  return;
}

void fragment_main() {
  textureDimensions_70e26a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_70e26a();
  return;
}

