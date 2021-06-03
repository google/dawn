RWTexture2DArray<uint4> arg_0 : register(u0, space1);

void textureNumLayers_cd5dc8() {
  int3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int res = tint_tmp.z;
}

void vertex_main() {
  textureNumLayers_cd5dc8();
  return;
}

void fragment_main() {
  textureNumLayers_cd5dc8();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumLayers_cd5dc8();
  return;
}

