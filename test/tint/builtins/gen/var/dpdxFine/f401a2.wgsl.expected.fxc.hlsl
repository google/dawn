RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdxFine_f401a2() {
  float arg_0 = 1.0f;
  float res = ddx_fine(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  dpdxFine_f401a2();
  return;
}
