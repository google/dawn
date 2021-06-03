RWTexture1D<float4> arg_0 : register(u0, space1);

void textureDimensions_1e9e39() {
  int tint_tmp;
  arg_0.GetDimensions(tint_tmp);
  int res = tint_tmp;
}

void vertex_main() {
  textureDimensions_1e9e39();
  return;
}

void fragment_main() {
  textureDimensions_1e9e39();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_1e9e39();
  return;
}

