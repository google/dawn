Texture2DArray arg_0 : register(t0, space1);

void textureLoad_9b2667() {
  float res = arg_0.Load(int4(0, 0, 1, 0), 1);
}

void vertex_main() {
  textureLoad_9b2667();
  return;
}

void fragment_main() {
  textureLoad_9b2667();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_9b2667();
  return;
}

