TextureCubeArray arg_0 : register(t0, space1);

void textureDimensions_bdf2ee() {
  int3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int3 res = tint_tmp.xyy;
}

void vertex_main() {
  textureDimensions_bdf2ee();
  return;
}

void fragment_main() {
  textureDimensions_bdf2ee();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_bdf2ee();
  return;
}

