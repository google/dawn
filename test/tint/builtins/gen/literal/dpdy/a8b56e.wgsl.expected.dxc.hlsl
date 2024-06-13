float2 dpdy_a8b56e() {
  float2 res = ddy((1.0f).xx);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store2(0u, asuint(dpdy_a8b56e()));
  return;
}
