void pack2x16snorm_6c169b() {
  int2 tint_tmp = int2(round(clamp(float2(0.0f, 0.0f), -1.0, 1.0) * 32767.0)) & 0xffff;
  uint res = asuint(tint_tmp.x | tint_tmp.y << 16);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  pack2x16snorm_6c169b();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  pack2x16snorm_6c169b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pack2x16snorm_6c169b();
  return;
}
