Texture2DArray<int4> arg_0 : register(t0, space1);

void textureLoad_a6b61d() {
  int4 res = arg_0.Load(int4(0, 0, 1, 0));
}

void vertex_main() {
  textureLoad_a6b61d();
  return;
}

void fragment_main() {
  textureLoad_a6b61d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_a6b61d();
  return;
}

