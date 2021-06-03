Texture2DArray<float4> arg_0 : register(t0, space1);

void textureLoad_20fa2f() {
  float4 res = arg_0.Load(int4(0, 0, 1, 0));
}

void vertex_main() {
  textureLoad_20fa2f();
  return;
}

void fragment_main() {
  textureLoad_20fa2f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_20fa2f();
  return;
}

