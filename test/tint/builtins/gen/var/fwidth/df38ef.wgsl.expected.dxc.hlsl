RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidth_df38ef() {
  float arg_0 = 1.0f;
  float res = fwidth(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  fwidth_df38ef();
  return;
}
