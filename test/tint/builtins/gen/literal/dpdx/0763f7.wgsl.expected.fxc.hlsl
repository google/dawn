RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdx_0763f7() {
  float3 res = ddx((1.0f).xxx);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  dpdx_0763f7();
  return;
}
