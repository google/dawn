Texture2DArray<uint4> arg_0 : register(t0, space1);

void textureNumLevels_5101cf() {
  int4 tint_tmp;
  arg_0.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
  int res = tint_tmp.w;
}

void vertex_main() {
  textureNumLevels_5101cf();
  return;
}

void fragment_main() {
  textureNumLevels_5101cf();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumLevels_5101cf();
  return;
}

