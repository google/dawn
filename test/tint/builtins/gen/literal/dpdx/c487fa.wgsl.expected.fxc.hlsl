RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdx_c487fa() {
  float4 res = ddx((1.0f).xxxx);
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  dpdx_c487fa();
  return;
}
