RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdxCoarse_029152() {
  float arg_0 = 1.0f;
  float res = ddx_coarse(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  dpdxCoarse_029152();
  return;
}
