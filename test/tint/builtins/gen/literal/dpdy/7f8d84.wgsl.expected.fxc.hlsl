RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdy_7f8d84() {
  float res = ddy(1.0f);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  dpdy_7f8d84();
  return;
}
