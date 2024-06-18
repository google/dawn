SKIP: FAILED

float2 dpdx_99edb1() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddx(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdx_99edb1();
}

