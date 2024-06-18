SKIP: FAILED

float dpdx_e263de() {
  float res = ddx(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = dpdx_e263de();
}

