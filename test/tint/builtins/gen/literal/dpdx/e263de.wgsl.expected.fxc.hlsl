RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdx_e263de() {
  float res = ddx(1.0f);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  dpdx_e263de();
  return;
}
