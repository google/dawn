Texture2DMS<int4> arg_0 : register(t0, space1);

void textureNumSamples_449d23() {
  int3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int res = tint_tmp.z;
}

void vertex_main() {
  textureNumSamples_449d23();
  return;
}

void fragment_main() {
  textureNumSamples_449d23();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumSamples_449d23();
  return;
}

