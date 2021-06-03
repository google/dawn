Texture2DArray<uint4> arg_0 : register(t0, space1);

void textureLoad_fdebd0() {
  uint4 res = arg_0.Load(int4(0, 0, 1, 0));
}

void vertex_main() {
  textureLoad_fdebd0();
  return;
}

void fragment_main() {
  textureLoad_fdebd0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_fdebd0();
  return;
}

