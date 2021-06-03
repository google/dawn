void pack2x16unorm_0f08e4() {
  uint2 tint_tmp = uint2(round(clamp(float2(0.0f, 0.0f), 0.0, 1.0) * 65535.0));
uint res = (tint_tmp.x | tint_tmp.y << 16);
}

void vertex_main() {
  pack2x16unorm_0f08e4();
  return;
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

