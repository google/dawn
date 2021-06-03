Texture1D<int4> arg_0 : register(t0, space1);

void textureLoad_5a2f9d() {
  int4 res = arg_0.Load(int2(1, 0), 1);
}

void vertex_main() {
  textureLoad_5a2f9d();
  return;
}

void fragment_main() {
  textureLoad_5a2f9d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_5a2f9d();
  return;
}

