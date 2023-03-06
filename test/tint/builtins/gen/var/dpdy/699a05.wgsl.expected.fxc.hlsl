RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdy_699a05() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = ddy(arg_0);
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  dpdy_699a05();
  return;
}
