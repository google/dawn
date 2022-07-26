void dpdx_99edb1() {
  float2 res = ddx((1.0f).xx);
}

void fragment_main() {
  dpdx_99edb1();
  return;
}
