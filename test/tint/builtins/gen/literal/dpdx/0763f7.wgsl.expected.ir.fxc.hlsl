SKIP: FAILED

float3 dpdx_0763f7() {
  float3 res = ddx((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdx_0763f7();
}

