SKIP: FAILED

float fwidthFine_f1742d() {
  float res = fwidth(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthFine_f1742d();
}

