void pack2x16snorm_6c169b() {
  int2 tint_tmp = int2(round(clamp(float2(0.0f, 0.0f), -1.0, 1.0) * 32767.0)) & 0xffff;
uint res = asuint(tint_tmp.x | tint_tmp.y << 16);
}

void vertex_main() {
  pack2x16snorm_6c169b();
  return;
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

