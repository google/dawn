TextureCubeArray arg_0 : register(t0, space1);

void textureNumLevels_2c3575() {
  int4 tint_tmp;
  arg_0.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
  int res = tint_tmp.w;
}

void vertex_main() {
  textureNumLevels_2c3575();
  return;
}

void fragment_main() {
  textureNumLevels_2c3575();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumLevels_2c3575();
  return;
}

