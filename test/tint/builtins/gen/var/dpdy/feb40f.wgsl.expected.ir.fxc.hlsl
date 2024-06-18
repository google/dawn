SKIP: FAILED

float3 dpdy_feb40f() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddy(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_feb40f();
}

