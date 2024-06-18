SKIP: FAILED

float2 dpdy_a8b56e() {
  float2 res = ddy((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_a8b56e();
}

