RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdyCoarse_3e1ab4() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddy_coarse(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

void fragment_main() {
  dpdyCoarse_3e1ab4();
  return;
}
