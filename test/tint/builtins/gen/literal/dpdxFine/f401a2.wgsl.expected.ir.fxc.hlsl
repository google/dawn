SKIP: FAILED

float dpdxFine_f401a2() {
  float res = ddx_fine(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxFine_f401a2();
}

