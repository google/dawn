void dpdy_feb40f() {
  float3 res = ddy((1.0f).xxx);
}

void fragment_main() {
  dpdy_feb40f();
  return;
}
