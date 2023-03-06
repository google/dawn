RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidth_df38ef() {
  float res = fwidth(1.0f);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  fwidth_df38ef();
  return;
}
