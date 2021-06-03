Texture3D<int4> arg_0 : register(t0, space1);

void textureLoad_a7a3c3() {
  int4 res = arg_0.Load(int4(0, 0, 0, 0));
}

void vertex_main() {
  textureLoad_a7a3c3();
  return;
}

void fragment_main() {
  textureLoad_a7a3c3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_a7a3c3();
  return;
}

