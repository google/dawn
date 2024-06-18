SKIP: FAILED

float4 dpdx_c487fa() {
  float4 res = ddx((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdx_c487fa();
}

