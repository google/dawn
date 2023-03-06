RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdx_99edb1() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddx(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

void fragment_main() {
  dpdx_99edb1();
  return;
}
