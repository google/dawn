TextureCube<int4> arg_0 : register(t0, space1);

void textureNumLevels_080d95() {
  int3 tint_tmp;
  arg_0.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int res = tint_tmp.z;
}

void vertex_main() {
  textureNumLevels_080d95();
  return;
}

void fragment_main() {
  textureNumLevels_080d95();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumLevels_080d95();
  return;
}

