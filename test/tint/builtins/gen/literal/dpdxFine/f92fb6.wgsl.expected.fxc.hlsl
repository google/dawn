RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdxFine_f92fb6() {
  float3 res = ddx_fine((1.0f).xxx);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  dpdxFine_f92fb6();
  return;
}
