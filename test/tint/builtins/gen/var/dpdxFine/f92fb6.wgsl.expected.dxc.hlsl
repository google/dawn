
RWByteAddressBuffer prevent_dce : register(u0);
float3 dpdxFine_f92fb6() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddx_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(dpdxFine_f92fb6()));
}

