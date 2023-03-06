RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdxCoarse_f64d7b() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddx_coarse(arg_0);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  dpdxCoarse_f64d7b();
  return;
}
