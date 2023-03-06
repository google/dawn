RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdxCoarse_029152() {
  float res = ddx_coarse(1.0f);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  dpdxCoarse_029152();
  return;
}
