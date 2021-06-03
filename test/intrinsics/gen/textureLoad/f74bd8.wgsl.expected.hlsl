Texture3D<float4> arg_0 : register(t0, space1);

void textureLoad_f74bd8() {
  float4 res = arg_0.Load(int4(0, 0, 0, 0));
}

void vertex_main() {
  textureLoad_f74bd8();
  return;
}

void fragment_main() {
  textureLoad_f74bd8();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_f74bd8();
  return;
}

