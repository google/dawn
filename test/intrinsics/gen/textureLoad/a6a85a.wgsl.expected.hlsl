Texture3D<float4> arg_0 : register(t0, space1);

void textureLoad_a6a85a() {
  float4 res = arg_0.Load(int4(0, 0, 0, 0));
}

void vertex_main() {
  textureLoad_a6a85a();
  return;
}

void fragment_main() {
  textureLoad_a6a85a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_a6a85a();
  return;
}

