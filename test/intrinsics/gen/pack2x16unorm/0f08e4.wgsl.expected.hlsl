void pack2x16unorm_0f08e4() {
  uint2 tint_tmp = uint2(round(clamp(float2(0.0f, 0.0f), 0.0, 1.0) * 65535.0));
  uint res = (tint_tmp.x | tint_tmp.y << 16);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  pack2x16unorm_0f08e4();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  pack2x16unorm_0f08e4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pack2x16unorm_0f08e4();
  return;
}
