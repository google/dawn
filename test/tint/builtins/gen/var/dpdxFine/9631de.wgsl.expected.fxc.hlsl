RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdxFine_9631de() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddx_fine(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

void fragment_main() {
  dpdxFine_9631de();
  return;
}
