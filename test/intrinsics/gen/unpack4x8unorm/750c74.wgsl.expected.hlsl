void unpack4x8unorm_750c74() {
  uint tint_tmp_1 = 1u;
uint4 tint_tmp = uint4(tint_tmp_1 & 0xff, (tint_tmp_1 >> 8) & 0xff, (tint_tmp_1 >> 16) & 0xff, tint_tmp_1 >> 24);
float4 res = float4(tint_tmp) / 255.0;
}

void vertex_main() {
  unpack4x8unorm_750c74();
  return;
}

void fragment_main() {
  unpack4x8unorm_750c74();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack4x8unorm_750c74();
  return;
}

