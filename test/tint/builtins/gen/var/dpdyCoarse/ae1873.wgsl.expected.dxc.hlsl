RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdyCoarse_ae1873() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddy_coarse(arg_0);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  dpdyCoarse_ae1873();
  return;
}
