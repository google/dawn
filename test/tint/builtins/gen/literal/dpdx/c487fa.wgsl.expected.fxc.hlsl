void dpdx_c487fa() {
  float4 res = ddx((0.0f).xxxx);
}

void fragment_main() {
  dpdx_c487fa();
  return;
}
