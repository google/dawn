uint tint_pack4x8snorm(float4 param_0) {
  int4 i = int4(round(clamp(param_0, -1.0, 1.0) * 127.0)) & 0xff;
  return asuint(i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

void pack4x8snorm_4d22e7() {
  uint res = tint_pack4x8snorm(float4(0.0f, 0.0f, 0.0f, 0.0f));
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
