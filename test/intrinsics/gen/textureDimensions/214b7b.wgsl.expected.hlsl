Texture1D<float4> arg_0 : register(t0, space1);

void textureDimensions_214b7b() {
  int tint_tmp;
  arg_0.GetDimensions(tint_tmp);
  int res = tint_tmp;
}

void vertex_main() {
  textureDimensions_214b7b();
  return;
}

void fragment_main() {
  textureDimensions_214b7b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_214b7b();
  return;
}

