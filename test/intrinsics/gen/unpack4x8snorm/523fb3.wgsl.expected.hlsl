float4 tint_unpack4x8snorm(uint param_0) {
  int j = int(param_0);
  int4 i = int4(j << 24, j << 16, j << 8, j) >> 24;
  return clamp(float4(i) / 127.0, -1.0, 1.0);
}

void unpack4x8snorm_523fb3() {
  float4 res = tint_unpack4x8snorm(1u);
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
