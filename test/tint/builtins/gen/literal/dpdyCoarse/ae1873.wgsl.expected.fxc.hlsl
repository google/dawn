RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdyCoarse_ae1873() {
  float3 res = ddy_coarse((1.0f).xxx);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  dpdyCoarse_ae1873();
  return;
}
