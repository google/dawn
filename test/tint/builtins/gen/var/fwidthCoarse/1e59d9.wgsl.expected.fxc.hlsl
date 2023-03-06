RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidthCoarse_1e59d9() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = fwidth(arg_0);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  fwidthCoarse_1e59d9();
  return;
}
