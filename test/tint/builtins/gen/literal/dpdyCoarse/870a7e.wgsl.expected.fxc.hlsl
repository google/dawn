RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdyCoarse_870a7e() {
  float res = ddy_coarse(1.0f);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  dpdyCoarse_870a7e();
  return;
}
