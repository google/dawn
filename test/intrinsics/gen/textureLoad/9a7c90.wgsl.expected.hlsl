Texture3D<uint4> arg_0 : register(t0, space1);

void textureLoad_9a7c90() {
  uint4 res = arg_0.Load(int4(0, 0, 0, 0));
}

void vertex_main() {
  textureLoad_9a7c90();
  return;
}

void fragment_main() {
  textureLoad_9a7c90();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_9a7c90();
  return;
}

