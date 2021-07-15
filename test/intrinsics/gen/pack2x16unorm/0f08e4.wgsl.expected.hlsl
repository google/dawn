uint tint_pack2x16unorm(float2 param_0) {
  uint2 i = uint2(round(clamp(param_0, 0.0, 1.0) * 65535.0));
  return (i.x | i.y << 16);
}

void pack2x16unorm_0f08e4() {
  uint res = tint_pack2x16unorm(float2(0.0f, 0.0f));
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
