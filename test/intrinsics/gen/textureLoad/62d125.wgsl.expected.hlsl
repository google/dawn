Texture3D<float4> arg_0 : register(t0, space1);

void textureLoad_62d125() {
  float4 res = arg_0.Load(int4(0, 0, 0, 0));
}

void vertex_main() {
  textureLoad_62d125();
  return;
}

void fragment_main() {
  textureLoad_62d125();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_62d125();
  return;
}

