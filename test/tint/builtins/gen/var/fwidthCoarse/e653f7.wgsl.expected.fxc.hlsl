RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidthCoarse_e653f7() {
  float2 arg_0 = (1.0f).xx;
  float2 res = fwidth(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

void fragment_main() {
  fwidthCoarse_e653f7();
  return;
}
