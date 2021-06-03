Texture2D<uint4> arg_0 : register(t0, space1);

void textureNumLevels_09ddd0() {
  int3 tint_tmp;
  arg_0.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int res = tint_tmp.z;
}

void vertex_main() {
  textureNumLevels_09ddd0();
  return;
}

void fragment_main() {
  textureNumLevels_09ddd0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumLevels_09ddd0();
  return;
}

