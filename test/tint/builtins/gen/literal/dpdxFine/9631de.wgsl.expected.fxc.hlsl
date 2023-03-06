RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdxFine_9631de() {
  float2 res = ddx_fine((1.0f).xx);
  prevent_dce.Store2(0u, asuint(res));
}

void fragment_main() {
  dpdxFine_9631de();
  return;
}
