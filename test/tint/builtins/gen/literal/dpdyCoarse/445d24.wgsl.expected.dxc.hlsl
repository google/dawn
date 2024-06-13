float4 dpdyCoarse_445d24() {
  float4 res = ddy_coarse((1.0f).xxxx);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store4(0u, asuint(dpdyCoarse_445d24()));
  return;
}
