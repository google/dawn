RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidthFine_68f4ef() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = fwidth(arg_0);
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  fwidthFine_68f4ef();
  return;
}
