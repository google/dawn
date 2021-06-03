Texture2DMS<uint4> arg_0 : register(t0, space1);

void textureLoad_c378ee() {
  uint4 res = arg_0.Load(int3(0, 0, 0), 1);
}

void vertex_main() {
  textureLoad_c378ee();
  return;
}

void fragment_main() {
  textureLoad_c378ee();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_c378ee();
  return;
}

