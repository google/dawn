RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdy_a8b56e() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddy(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

void fragment_main() {
  dpdy_a8b56e();
  return;
}
