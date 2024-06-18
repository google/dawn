SKIP: FAILED

float3 dpdy_feb40f() {
  float3 res = ddy((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_feb40f();
}

