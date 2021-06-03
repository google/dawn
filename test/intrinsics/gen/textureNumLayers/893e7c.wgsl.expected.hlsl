Texture2DArray<int4> arg_0 : register(t0, space1);

void textureNumLayers_893e7c() {
  int3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int res = tint_tmp.z;
}

void vertex_main() {
  textureNumLayers_893e7c();
  return;
}

void fragment_main() {
  textureNumLayers_893e7c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumLayers_893e7c();
  return;
}

