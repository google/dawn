SKIP: FAILED

float2 dpdx_99edb1() {
  float2 res = ddx((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdx_99edb1();
}

