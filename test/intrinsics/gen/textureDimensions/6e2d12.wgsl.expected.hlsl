Texture1D<int4> arg_0 : register(t0, space1);

void textureDimensions_6e2d12() {
  int tint_tmp;
  arg_0.GetDimensions(tint_tmp);
  int res = tint_tmp;
}

void vertex_main() {
  textureDimensions_6e2d12();
  return;
}

void fragment_main() {
  textureDimensions_6e2d12();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_6e2d12();
  return;
}

