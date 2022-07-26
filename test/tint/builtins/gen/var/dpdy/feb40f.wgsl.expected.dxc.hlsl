void dpdy_feb40f() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddy(arg_0);
}

void fragment_main() {
  dpdy_feb40f();
  return;
}
