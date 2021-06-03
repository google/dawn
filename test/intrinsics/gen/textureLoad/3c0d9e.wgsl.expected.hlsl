Texture2D<uint4> arg_0 : register(t0, space1);

void textureLoad_3c0d9e() {
  uint4 res = arg_0.Load(int3(0, 0, 0));
}

void vertex_main() {
  textureLoad_3c0d9e();
  return;
}

void fragment_main() {
  textureLoad_3c0d9e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_3c0d9e();
  return;
}

