void pack4x8snorm_4d22e7() {
  int4 tint_tmp = int4(round(clamp(float4(0.0f, 0.0f, 0.0f, 0.0f), -1.0, 1.0) * 127.0)) & 0xff;
  uint res = asuint(tint_tmp.x | tint_tmp.y << 8 | tint_tmp.z << 16 | tint_tmp.w << 24);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  pack4x8snorm_4d22e7();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  pack4x8snorm_4d22e7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pack4x8snorm_4d22e7();
  return;
}
