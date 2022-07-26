void dpdy_a8b56e() {
  float2 res = ddy((1.0f).xx);
}

void fragment_main() {
  dpdy_a8b56e();
  return;
}
