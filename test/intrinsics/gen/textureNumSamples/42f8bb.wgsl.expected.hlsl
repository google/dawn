Texture2DMS<uint4> arg_0 : register(t0, space1);

void textureNumSamples_42f8bb() {
  int3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int res = tint_tmp.z;
}

void vertex_main() {
  textureNumSamples_42f8bb();
  return;
}

void fragment_main() {
  textureNumSamples_42f8bb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumSamples_42f8bb();
  return;
}

