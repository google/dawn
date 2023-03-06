RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdyFine_df33aa() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddy_fine(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

void fragment_main() {
  dpdyFine_df33aa();
  return;
}
