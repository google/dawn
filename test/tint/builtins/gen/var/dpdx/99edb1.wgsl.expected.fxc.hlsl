void dpdx_99edb1() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddx(arg_0);
}

void fragment_main() {
  dpdx_99edb1();
  return;
}
