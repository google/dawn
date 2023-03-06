RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdy_7f8d84() {
  float arg_0 = 1.0f;
  float res = ddy(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  dpdy_7f8d84();
  return;
}
