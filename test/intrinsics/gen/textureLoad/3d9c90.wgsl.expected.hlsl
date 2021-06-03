Texture3D<float4> arg_0 : register(t0, space1);

void textureLoad_3d9c90() {
  float4 res = arg_0.Load(int4(0, 0, 0, 0));
}

void vertex_main() {
  textureLoad_3d9c90();
  return;
}

void fragment_main() {
  textureLoad_3d9c90();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_3d9c90();
  return;
}

