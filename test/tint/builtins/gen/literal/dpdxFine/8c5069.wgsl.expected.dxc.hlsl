RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdxFine_8c5069() {
  float4 res = ddx_fine((1.0f).xxxx);
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  dpdxFine_8c5069();
  return;
}
