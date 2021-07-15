uint tint_pack2x16snorm(float2 param_0) {
  int2 i = int2(round(clamp(param_0, -1.0, 1.0) * 32767.0)) & 0xffff;
  return asuint(i.x | i.y << 16);
}

void pack2x16snorm_6c169b() {
  uint res = tint_pack2x16snorm(float2(0.0f, 0.0f));
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
