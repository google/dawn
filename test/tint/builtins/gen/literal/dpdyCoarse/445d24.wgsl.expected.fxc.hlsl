RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdyCoarse_445d24() {
  float4 res = ddy_coarse((1.0f).xxxx);
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  dpdyCoarse_445d24();
  return;
}
