Texture2DArray arg_0 : register(t0, space1);

void textureNumLevels_f5828d() {
  int4 tint_tmp;
  arg_0.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
  int res = tint_tmp.w;
}

void vertex_main() {
  textureNumLevels_f5828d();
  return;
}

void fragment_main() {
  textureNumLevels_f5828d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumLevels_f5828d();
  return;
}

