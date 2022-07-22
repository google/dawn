void dpdx_0763f7() {
  float3 res = ddx((0.0f).xxx);
}

void fragment_main() {
  dpdx_0763f7();
  return;
}
