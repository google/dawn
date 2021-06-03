Texture2D arg_0 : register(t0, space1);

void textureLoad_19cf87() {
  float res = arg_0.Load(int3(0, 0, 0), 1);
}

void vertex_main() {
  textureLoad_19cf87();
  return;
}

void fragment_main() {
  textureLoad_19cf87();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_19cf87();
  return;
}

