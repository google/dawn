float dpdx_e263de() {
  float arg_0 = 1.0f;
  float res = ddx(arg_0);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store(0u, asuint(dpdx_e263de()));
  return;
}
