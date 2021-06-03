Texture2DMS<int4> arg_0 : register(t0, space1);

void textureLoad_e3d2cc() {
  int4 res = arg_0.Load(int3(0, 0, 0), 1);
}

void vertex_main() {
  textureLoad_e3d2cc();
  return;
}

void fragment_main() {
  textureLoad_e3d2cc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_e3d2cc();
  return;
}

