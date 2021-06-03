Texture3D<uint4> arg_0 : register(t0, space1);

void textureLoad_a9a9f5() {
  uint4 res = arg_0.Load(int4(0, 0, 0, 0), 1);
}

void vertex_main() {
  textureLoad_a9a9f5();
  return;
}

void fragment_main() {
  textureLoad_a9a9f5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_a9a9f5();
  return;
}

