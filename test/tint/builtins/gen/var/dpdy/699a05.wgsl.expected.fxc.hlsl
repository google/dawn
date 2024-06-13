float4 dpdy_699a05() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = ddy(arg_0);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store4(0u, asuint(dpdy_699a05()));
  return;
}
