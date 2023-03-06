RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdyCoarse_870a7e() {
  float arg_0 = 1.0f;
  float res = ddy_coarse(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  dpdyCoarse_870a7e();
  return;
}
