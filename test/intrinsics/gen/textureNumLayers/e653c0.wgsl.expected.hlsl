Texture2DArray arg_0 : register(t0, space1);

void textureNumLayers_e653c0() {
  int3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int res = tint_tmp.z;
}

void vertex_main() {
  textureNumLayers_e653c0();
  return;
}

void fragment_main() {
  textureNumLayers_e653c0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumLayers_e653c0();
  return;
}

