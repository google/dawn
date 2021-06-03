Texture3D<int4> arg_0 : register(t0, space1);

void textureLoad_4fd803() {
  int4 res = arg_0.Load(int4(0, 0, 0, 0), 1);
}

void vertex_main() {
  textureLoad_4fd803();
  return;
}

void fragment_main() {
  textureLoad_4fd803();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_4fd803();
  return;
}

