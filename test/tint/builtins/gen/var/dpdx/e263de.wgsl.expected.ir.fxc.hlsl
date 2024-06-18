SKIP: FAILED

float dpdx_e263de() {
  float arg_0 = 1.0f;
  float res = ddx(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdx_e263de();
}

