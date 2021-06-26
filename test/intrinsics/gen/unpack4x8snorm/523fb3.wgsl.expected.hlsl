void unpack4x8snorm_523fb3() {
  int tint_tmp_1 = int(1u);
  int4 tint_tmp = int4(tint_tmp_1 << 24, tint_tmp_1 << 16, tint_tmp_1 << 8, tint_tmp_1) >> 24;
  float4 res = clamp(float4(tint_tmp) / 127.0, -1.0, 1.0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  unpack4x8snorm_523fb3();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  unpack4x8snorm_523fb3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack4x8snorm_523fb3();
  return;
}
