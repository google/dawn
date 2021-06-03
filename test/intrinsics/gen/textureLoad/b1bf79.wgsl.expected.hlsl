Texture3D<int4> arg_0 : register(t0, space1);

void textureLoad_b1bf79() {
  int4 res = arg_0.Load(int4(0, 0, 0, 0));
}

void vertex_main() {
  textureLoad_b1bf79();
  return;
}

void fragment_main() {
  textureLoad_b1bf79();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_b1bf79();
  return;
}

