SKIP: FAILED

float2 dpdy_a8b56e() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddy(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_a8b56e();
}

