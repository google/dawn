void dpdx_99edb1() {
  float2 res = ddx((0.0f).xx);
}

void fragment_main() {
  dpdx_99edb1();
  return;
}
